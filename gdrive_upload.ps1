# STEM VCS - Google Drive Upload Script
# Uploads repository files to Google Drive folder
# 
# Usage:
#   .\gdrive_upload.ps1 -SourcePath ".stem/objects" -FolderId "1NYTBuwkKVaMF2qfojHRwzTXzPIpELQxz"
#
# First time setup:
#   1. Run: .\gdrive_upload.ps1 -SetupAuth
#   2. Follow browser prompts to authorize
#   3. Access token saved to .stem/gdrive_token
#
# Copyright 2026 STEM VCS

param(
    [string]$SourcePath = "",
    [string]$FolderId = "",
    [string]$AccessToken = "",
    [string]$DestinationPath = "",
    [int]$KeepBackups = 0,
    [switch]$SetupAuth = $false,
    [switch]$RefreshToken = $false,
    [switch]$Pull = $false
)

$ErrorActionPreference = "Stop"
$VerbosePreference = "SilentlyContinue"

# Google Drive API Configuration
$ClientId = $env:STEM_GDRIVE_CLIENT_ID
$ClientSecret = $env:STEM_GDRIVE_CLIENT_SECRET
# For Desktop apps, Google uses this special redirect URI
$RedirectUri = "http://localhost"
$Scope = "https://www.googleapis.com/auth/drive.file"

# Token file location - store in user's home directory so it works from anywhere
$TokenFile = "$env:USERPROFILE\.stem_gdrive_token"
$script:LastUploadAuthError = $false

function Write-Rule {
    Write-Host "------------------------------------------------------------" -ForegroundColor DarkGray
}

function Write-Title {
    param([string]$Text)
    Write-Host ""
    Write-Rule
    Write-Host ("  {0}" -f $Text) -ForegroundColor Cyan
    Write-Rule
}

function Write-Phase {
    param([int]$Number, [string]$Text)
    Write-Host ""
    Write-Host ("Phase {0}: {1}" -f $Number, $Text) -ForegroundColor Cyan
}

function Write-Note {
    param([string]$Text)
    Write-Host ("  {0}" -f $Text) -ForegroundColor DarkGray
}

function Format-Duration {
    param([double]$Seconds)

    if ($Seconds -lt 0) { $Seconds = 0 }
    $ts = [TimeSpan]::FromSeconds([math]::Round($Seconds, 0))

    if ($ts.TotalHours -ge 1) {
        return ("{0:D2}:{1:D2}:{2:D2}" -f $ts.Hours, $ts.Minutes, $ts.Seconds)
    }

    return ("{0:D2}:{1:D2}" -f $ts.Minutes, $ts.Seconds)
}

function Write-UploadProgress {
    param(
        [int64]$BytesSent,
        [int64]$TotalBytes,
        [datetime]$StartTime
    )

    if ($TotalBytes -le 0) {
        return
    }

    $percent = [math]::Min(100, [math]::Round(($BytesSent * 100.0) / $TotalBytes, 1))
    $elapsedSec = ((Get-Date) - $StartTime).TotalSeconds
    $sentMB = [math]::Round($BytesSent / 1MB, 2)
    $totalMB = [math]::Round($TotalBytes / 1MB, 2)

    $speedMBps = 0
    if ($elapsedSec -gt 0) {
        $speedMBps = [math]::Round(($BytesSent / 1MB) / $elapsedSec, 2)
    }

    $etaText = "--:--"
    if ($speedMBps -gt 0) {
        $remainingMB = ($TotalBytes - $BytesSent) / 1MB
        $etaSec = $remainingMB / $speedMBps
        $etaText = Format-Duration -Seconds $etaSec
    }

    $barWidth = 24
    $filled = [int][math]::Floor(($percent / 100) * $barWidth)
    if ($filled -lt 0) { $filled = 0 }
    if ($filled -gt $barWidth) { $filled = $barWidth }

    $bar = ("#" * $filled) + ("-" * ($barWidth - $filled))

    $line = ("`r  Uploading: [{0}] {1,6}%   {2,7} / {3,7} MB   {4,5} MB/s   ETA {5}" -f $bar, $percent, $sentMB, $totalMB, $speedMBps, $etaText)
    Write-Host $line -NoNewline -ForegroundColor DarkGray

    if ($BytesSent -ge $TotalBytes) {
        Write-Host ""
    }
}

function Initialize-CompressionSupport {
    # PowerShell 5.1 may not preload the assembly that defines ZipArchiveMode.
    # Load both assemblies so zip create/extract works reliably across hosts.
    Add-Type -AssemblyName System.IO.Compression
    Add-Type -AssemblyName System.IO.Compression.FileSystem
}

function Get-AuthorizationCode {
    if ([string]::IsNullOrWhiteSpace($ClientId)) {
        Write-Host "[ERROR] Set the STEM_GDRIVE_CLIENT_ID environment variable before authorization." -ForegroundColor Red
        return $null
    }

    Write-Host ""
    Write-Host "================================================================" -ForegroundColor Cyan
    Write-Host "                 STEM VCS - Google Drive Setup                  " -ForegroundColor Cyan
    Write-Host "================================================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "This is a ONE-TIME setup to authorize STEM to upload to Google Drive." -ForegroundColor White
    Write-Host ""
    Write-Host "What happens next:" -ForegroundColor Yellow
    Write-Host "  1. A temporary local web server starts (port 8080)"
    Write-Host "  2. Your browser opens Google's sign-in page"
    Write-Host "  3. You sign in and click 'Allow'"
    Write-Host "  4. Google redirects back to STEM automatically"
    Write-Host "  5. Setup complete - token saved for future use"
    Write-Host ""
    Write-Host "================================================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Press ENTER to continue..." -ForegroundColor Yellow -NoNewline
    Read-Host
    
    Write-Host ""
    Write-Host "[*] Starting local authentication server..." -ForegroundColor Cyan
    
    # Start a simple HTTP listener on localhost
    $listener = New-Object System.Net.HttpListener
    $listener.Prefixes.Add("http://localhost:8080/")
    $listener.Start()
    
    Write-Host "[OK] Server running on http://localhost:8080" -ForegroundColor Green
    
    # Update redirect URI to use port 8080
    $script:RedirectUri = "http://localhost:8080/"
    
    $authUrl = "https://accounts.google.com/o/oauth2/v2/auth?client_id=$ClientId&redirect_uri=$($script:RedirectUri)&response_type=code&scope=$Scope&access_type=offline&prompt=consent"
    
    Write-Host "[*] Opening browser for authorization..." -ForegroundColor Cyan
    Start-Process $authUrl
    
    Write-Host ""
    Write-Host "Waiting for authorization..." -ForegroundColor Yellow
    Write-Host "(Your browser will redirect automatically after clicking Allow)" -ForegroundColor Gray
    Write-Host ""
    
    # Wait for the OAuth callback
    $context = $listener.GetContext()
    $code = $context.Request.QueryString["code"]
    
    # Trim any whitespace from the code
    if ($code) {
        $code = $code.Trim()
    }
    
    # Send a response to the browser
    $response = $context.Response
    $responseString = @"
<!DOCTYPE html>
<html>
<head>
    <title>STEM VCS - Authorization Successful</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; padding: 50px; background: #f5f5f5; }
        .container { background: white; padding: 40px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); max-width: 500px; margin: 0 auto; }
        h1 { color: #4CAF50; }
        p { color: #666; line-height: 1.6; }
    </style>
</head>
<body>
    <div class="container">
        <h1>✓ Authorization Successful!</h1>
        <p>STEM VCS has been authorized to upload to Google Drive.</p>
        <p>You can now close this window and return to your terminal.</p>
    </div>
</body>
</html>
"@
    $buffer = [System.Text.Encoding]::UTF8.GetBytes($responseString)
    $response.ContentLength64 = $buffer.Length
    $response.OutputStream.Write($buffer, 0, $buffer.Length)
    $response.OutputStream.Close()
    
    $listener.Stop()
    
    if ($code) {
        Write-Host "[OK] Authorization code received" -ForegroundColor Green
    } else {
        Write-Host "[ERROR] No authorization code received" -ForegroundColor Red
    }
    
    return $code
}

function Get-AccessTokenFromCode {
    param([string]$Code)

    if ([string]::IsNullOrWhiteSpace($ClientId) -or [string]::IsNullOrWhiteSpace($ClientSecret)) {
        Write-Host "[ERROR] Set STEM_GDRIVE_CLIENT_ID and STEM_GDRIVE_CLIENT_SECRET before authorization." -ForegroundColor Red
        return $null
    }
    
    # Trim whitespace from code
    $Code = $Code.Trim()
    
    Write-Host "[*] Exchanging authorization code for access token..." -ForegroundColor Cyan
    
    $tokenUrl = "https://oauth2.googleapis.com/token"
    $body = @{
        code = $Code
        client_id = $ClientId
        client_secret = $ClientSecret
        redirect_uri = "http://localhost:8080/"
        grant_type = "authorization_code"
    }
    
    try {
        $response = Invoke-RestMethod -Uri $tokenUrl -Method Post -Body $body -ContentType "application/x-www-form-urlencoded"
        return $response
    } catch {
        Write-Host ""
        Write-Host "[ERROR] Failed to get access token" -ForegroundColor Red
        Write-Host ""
        Write-Host "This usually means:" -ForegroundColor Yellow
        Write-Host "  - The authorization code expired (try again quickly)"
        Write-Host "  - OAuth configuration issue"
        Write-Host ""
        Write-Host "Run setup again: .\gdrive_upload.ps1 -SetupAuth" -ForegroundColor Green
        return $null
    }
}

function Save-Token {
    param($TokenData)
    
    $tokenDir = Split-Path -Parent $TokenFile
    if (-not (Test-Path $tokenDir)) {
        New-Item -ItemType Directory -Path $tokenDir -Force | Out-Null
    }
    
    $TokenData | ConvertTo-Json | Set-Content -Path $TokenFile
    Write-Host "[OK] Authorization saved successfully" -ForegroundColor Green
}

function Load-Token {
    if (Test-Path $TokenFile) {
        return Get-Content -Path $TokenFile | ConvertFrom-Json
    }
    return $null
}

function Refresh-AccessToken {
    param([string]$RefreshToken)

    if ([string]::IsNullOrWhiteSpace($ClientId) -or [string]::IsNullOrWhiteSpace($ClientSecret)) {
        Write-Host "[ERROR] Set STEM_GDRIVE_CLIENT_ID and STEM_GDRIVE_CLIENT_SECRET before refreshing authorization." -ForegroundColor Red
        return $null
    }
    
    $tokenUrl = "https://oauth2.googleapis.com/token"
    $body = @{
        refresh_token = $RefreshToken
        client_id = $ClientId
        client_secret = $ClientSecret
        grant_type = "refresh_token"
    }
    
    try {
        $response = Invoke-RestMethod -Uri $tokenUrl -Method Post -Body $body -ContentType "application/x-www-form-urlencoded"
        return $response.access_token
    } catch {
        Write-Error "Failed to refresh token: $_"
        return $null
    }
}

function Upload-File {
    param(
        [string]$FilePath,
        [string]$ParentFolderId,
        [string]$Token
    )
    
    $fileName = Split-Path -Leaf $FilePath
        $localSize = (Get-Item -LiteralPath $FilePath).Length
        $localSizeMB = [math]::Round($localSize / 1MB, 2)
        $fileId = $null
    
    # First, create the file metadata
    $metadata = @{
        name = $fileName
        parents = @($ParentFolderId)
    } | ConvertTo-Json

    $headers = @{ "Authorization" = "Bearer $Token" }
    
    $script:LastUploadAuthError = $false

    try {
        Write-Host "  1/3 Start resumable upload session" -ForegroundColor Gray

        # Create resumable session (includes metadata)
        $sessionHeaders = @{
            "Authorization" = "Bearer $Token"
            "X-Upload-Content-Type" = "application/octet-stream"
            "X-Upload-Content-Length" = "$localSize"
        }

        $sessionResponse = Invoke-WebRequest -Uri "https://www.googleapis.com/upload/drive/v3/files?uploadType=resumable" `
            -Method Post `
            -Headers $sessionHeaders `
            -Body $metadata `
            -ContentType "application/json; charset=UTF-8" `
            -TimeoutSec 30 `
            -UseBasicParsing `
            -Verbose:$false

        $uploadUrl = $sessionResponse.Headers["Location"]
        if (-not $uploadUrl) {
            throw "Failed to initialize resumable upload session (missing upload URL)."
        }

        Write-Host "  OK  Upload session ready" -ForegroundColor Green

        $maxAttempts = 3
        $uploadTimeoutSec = 900
        $chunkSize = 8MB

        Write-Host "  2/3 Upload archive content" -ForegroundColor Gray

        for ($attempt = 1; $attempt -le $maxAttempts; $attempt++) {
            $fileStream = $null
            try {
                $bytesSent = 0L
                $finalJson = $null
                $uploadStart = Get-Date

                $fileStream = [System.IO.File]::OpenRead($FilePath)
                $buffer = New-Object byte[] $chunkSize

                while ($bytesSent -lt $localSize) {
                    $remaining = $localSize - $bytesSent
                    $toRead = [int][math]::Min($chunkSize, $remaining)
                    $read = $fileStream.Read($buffer, 0, $toRead)

                    if ($read -le 0) {
                        break
                    }

                    $startByte = $bytesSent
                    $endByte = $bytesSent + $read - 1

                    $request = [System.Net.HttpWebRequest]::Create($uploadUrl)
                    $request.Method = "PUT"
                    $request.Timeout = $uploadTimeoutSec * 1000
                    $request.ReadWriteTimeout = $uploadTimeoutSec * 1000
                    $request.AllowWriteStreamBuffering = $false
                    $request.SendChunked = $false
                    $request.ContentType = "application/octet-stream"
                    $request.ContentLength = $read
                    $request.Headers["Authorization"] = "Bearer $Token"
                    $request.Headers["Content-Range"] = "bytes $startByte-$endByte/$localSize"

                    $requestStream = $request.GetRequestStream()
                    try {
                        $requestStream.Write($buffer, 0, $read)
                    } finally {
                        $requestStream.Close()
                    }

                    try {
                        $response = $request.GetResponse()
                        try {
                            $statusCode = [int]$response.StatusCode
                            if ($statusCode -eq 200 -or $statusCode -eq 201) {
                                $bytesSent = $endByte + 1
                                Write-UploadProgress -BytesSent $bytesSent -TotalBytes $localSize -StartTime $uploadStart

                                $reader = New-Object System.IO.StreamReader($response.GetResponseStream())
                                $respText = $reader.ReadToEnd()
                                $reader.Close()

                                if ($respText) {
                                    $finalJson = $respText | ConvertFrom-Json
                                }
                                break
                            } else {
                                throw "Unexpected HTTP status: $statusCode"
                            }
                        } finally {
                            $response.Close()
                        }
                    } catch [System.Net.WebException] {
                        $webResp = $_.Exception.Response
                        if ($webResp) {
                            $respCode = [int]$webResp.StatusCode
                            try {
                                if ($respCode -eq 308) {
                                    $rangeHeader = $webResp.Headers["Range"]
                                    if ($rangeHeader -and $rangeHeader -match "bytes=0-(\d+)") {
                                        $bytesSent = [int64]$matches[1] + 1
                                    } else {
                                        $bytesSent = $endByte + 1
                                    }

                                    Write-UploadProgress -BytesSent $bytesSent -TotalBytes $localSize -StartTime $uploadStart
                                    continue
                                }
                            } finally {
                                $webResp.Close()
                            }
                        }

                        throw
                    }
                }

                if ($bytesSent -lt $localSize) {
                    throw "Upload ended before all bytes were sent ($bytesSent/$localSize)."
                }

                if ($finalJson -and $finalJson.id) {
                    $fileId = $finalJson.id
                }

                if (-not $fileId) {
                    throw "Upload finished but Drive file ID was not returned."
                }

                Write-Host "  3/3 Verify uploaded size" -ForegroundColor Gray

                $verifyHeaders = @{ "Authorization" = "Bearer $Token" }
                $verify = Invoke-RestMethod -Uri "https://www.googleapis.com/drive/v3/files/$fileId`?fields=id,name,size,webViewLink" `
                    -Method Get `
                    -Headers $verifyHeaders `
                    -TimeoutSec 30

                $remoteSize = [int64]0
                if ($verify.size) {
                    $remoteSize = [int64]$verify.size
                }

                if ($remoteSize -eq $localSize) {
                    return @{
                        id = $fileId
                        status = 200
                        size = $remoteSize
                        webViewLink = $verify.webViewLink
                    }
                }

                throw "Upload verification failed: remote size $remoteSize bytes does not match local size $localSize bytes."
            } catch {
                $statusCode = $null
                try {
                    if ($_.Exception.Response -and $_.Exception.Response.StatusCode) {
                        $statusCode = [int]$_.Exception.Response.StatusCode
                    }
                } catch {
                    $statusCode = $null
                }

                $isAuthError = ($statusCode -eq 401 -or $statusCode -eq 403)
                if ($isAuthError) {
                    Write-Host "[ERROR] Upload authorization failed (HTTP $statusCode)" -ForegroundColor Red
                    Write-Host "  Token may be expired or missing required permission." -ForegroundColor Yellow
                    return $null
                }

                if ($attempt -lt $maxAttempts) {
                    $delaySec = [math]::Pow(2, $attempt)
                    Write-Host ("  WARN Attempt {0}/{1} failed" -f $attempt, $maxAttempts) -ForegroundColor Yellow
                    Write-Note "Retrying in $delaySec second(s)..."
                    Start-Sleep -Seconds $delaySec
                } else {
                    throw
                }
            } finally {
                if ($fileStream) {
                    $fileStream.Close()
                    $fileStream.Dispose()
                }
            }
        }

        return $null
    } catch {
        $statusCode = $null
        try {
            if ($_.Exception.Response -and $_.Exception.Response.StatusCode) {
                $statusCode = [int]$_.Exception.Response.StatusCode
            }
        } catch {
            $statusCode = $null
        }

        if ($statusCode -eq 401 -or $statusCode -eq 403) {
            $script:LastUploadAuthError = $true
        }

        # If metadata was created but content upload/verification failed,
        # delete the incomplete remote file so users don't see 0-byte artifacts.
        if ($fileId) {
            try {
                $cleanupHeaders = @{ "Authorization" = "Bearer $Token" }
                Invoke-RestMethod -Uri "https://www.googleapis.com/drive/v3/files/$fileId" `
                    -Method Delete `
                    -Headers $cleanupHeaders `
                    -TimeoutSec 30 | Out-Null
                Write-Note "Removed incomplete remote file artifact."
            } catch {
                Write-Host "  WARN Could not remove incomplete remote file artifact." -ForegroundColor Yellow
            }
        }

        Write-Host "[ERROR] Upload failed for: $fileName" -ForegroundColor Red
        Write-Host "  Error: $($_.Exception.Message)" -ForegroundColor Yellow
        
        # Log detailed error
        $errorFile = "$env:TEMP\stem_upload_error.log"
        "=== Upload Error ===" | Out-File -FilePath $errorFile -Append
        "Time: $(Get-Date)" | Out-File -FilePath $errorFile -Append
        "File: $fileName" | Out-File -FilePath $errorFile -Append
        "Error: $($_.Exception.Message)" | Out-File -FilePath $errorFile -Append
        "" | Out-File -FilePath $errorFile -Append
        
        return $null
    }
}

function Create-FolderInDrive {
    param(
        [string]$FolderName,
        [string]$ParentFolderId,
        [string]$Token
    )
    
    $metadata = @{
        name = $FolderName
        mimeType = "application/vnd.google-apps.folder"
        parents = @($ParentFolderId)
    } | ConvertTo-Json
    
    $headers = @{
        "Authorization" = "Bearer $Token"
        "Content-Type" = "application/json"
    }
    
    try {
        $response = Invoke-RestMethod -Uri "https://www.googleapis.com/drive/v3/files" `
            -Method Post `
            -Headers $headers `
            -Body $metadata
        return $response.id
    } catch {
        return $null
    }
}

function Download-File {
    param(
        [string]$FileId,
        [string]$DestPath,
        [string]$Token
    )
    
    $headers = @{
        "Authorization" = "Bearer $Token"
    }
    
    try {
        $url = "https://www.googleapis.com/drive/v3/files/$FileId`?alt=media"
        Invoke-WebRequest -Uri $url -Headers $headers -OutFile $DestPath -TimeoutSec 300
        return $true
    } catch {
        Write-Host "[ERROR] Download failed: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

function Get-LatestBackupFile {
    param(
        [string]$FolderId,
        [string]$Token
    )
    
    $headers = @{
        "Authorization" = "Bearer $Token"
    }
    
    try {
        # List files in folder, sorted by modified time (newest first)
        $url = "https://www.googleapis.com/drive/v3/files?q='$FolderId'+in+parents+and+name+contains+'stem_backup'&orderBy=modifiedTime+desc&pageSize=1&fields=files(id,name,size,modifiedTime)"
        $response = Invoke-RestMethod -Uri $url -Headers $headers
        
        if ($response.files -and $response.files.Count -gt 0) {
            return $response.files[0]
        }
        return $null
    } catch {
        Write-Host "[ERROR] Failed to list files: $($_.Exception.Message)" -ForegroundColor Red
        return $null
    }
}

function Cleanup-OldBackups {
    param(
        [string]$FolderId,
        [string]$Token,
        [int]$KeepCount
    )

    if ($KeepCount -le 0) {
        return
    }

    $headers = @{ "Authorization" = "Bearer $Token" }
    $query = [System.Uri]::EscapeDataString("'$FolderId' in parents and name contains 'stem_backup' and trashed = false")
    $url = "https://www.googleapis.com/drive/v3/files?q=$query&orderBy=modifiedTime desc&pageSize=200&fields=files(id,name,size,modifiedTime)"

    try {
        $response = Invoke-RestMethod -Uri $url -Method Get -Headers $headers -TimeoutSec 60
        if (-not $response.files -or $response.files.Count -le $KeepCount) {
            return
        }

        $toDelete = @($response.files | Select-Object -Skip $KeepCount)
        if ($toDelete.Count -eq 0) {
            return
        }

        Write-Phase 4 "Retention cleanup"
        Write-Host "  Keeping latest $KeepCount backup(s), removing $($toDelete.Count) older backup(s)..." -ForegroundColor Gray

        foreach ($file in $toDelete) {
            try {
                Invoke-RestMethod -Uri "https://www.googleapis.com/drive/v3/files/$($file.id)" -Method Delete -Headers $headers -TimeoutSec 30 | Out-Null
                Write-Note "Removed old backup: $($file.name)"
            } catch {
                Write-Host "  WARN Failed to remove old backup: $($file.name)" -ForegroundColor Yellow
            }
        }
    } catch {
        Write-Host "  WARN Could not apply retention policy: $($_.Exception.Message)" -ForegroundColor Yellow
    }
}

# Main execution
if ($SetupAuth) {
    Write-Host "==================================================================" -ForegroundColor Cyan
    Write-Host "  STEM VCS - Google Drive Authorization Setup" -ForegroundColor Cyan
    Write-Host "==================================================================" -ForegroundColor Cyan
    
    $code = Get-AuthorizationCode
    if ($code) {
        $tokenData = Get-AccessTokenFromCode -Code $code
        if ($tokenData) {
            Save-Token -TokenData $tokenData
            Write-Host ""
            Write-Host "[OK] Authorization complete!" -ForegroundColor Green
            Write-Host ""
            Write-Host "You can now use 'stem push' to upload files to Google Drive." -ForegroundColor Green
        }
    }
    exit
}

# Load token
$tokenData = Load-Token
if (-not $tokenData) {
    Write-Host ""
    Write-Host "[ERROR] No authorization found" -ForegroundColor Red
    Write-Host ""
    Write-Host "Run this first:" -ForegroundColor Yellow
    Write-Host "  .\gdrive_upload.ps1 -SetupAuth" -ForegroundColor Yellow
    Write-Host ""
    exit 2
}

# Get access token (refresh if needed)
if ($AccessToken) {
    $token = $AccessToken
} elseif ($tokenData.refresh_token) {
    $token = Refresh-AccessToken -RefreshToken $tokenData.refresh_token
    if (-not $token) {
        Write-Host "[ERROR] Failed to refresh access token. Run setup again:" -ForegroundColor Red
        Write-Host "  .\gdrive_upload.ps1 -SetupAuth" -ForegroundColor Yellow
        exit 2
    }
} else {
    $token = $tokenData.access_token
}

# Pull/Download files
if ($Pull -and $FolderId -and $DestinationPath) {
    Write-Host ""
    Write-Host "================================================================" -ForegroundColor Cyan
    Write-Host "         Pulling from Google Drive (Compressed)                 " -ForegroundColor Cyan
    Write-Host "================================================================" -ForegroundColor Cyan
    Write-Host ""
    
    # Find latest backup file
    Write-Host "[*] Finding latest backup..." -ForegroundColor Cyan
    $backupFile = Get-LatestBackupFile -FolderId $FolderId -Token $token
    
    if (-not $backupFile) {
        Write-Host "[ERROR] No backup files found in Google Drive folder" -ForegroundColor Red
        exit 1
    }
    
    $fileSizeMB = [math]::Round($backupFile.size / 1MB, 2)
    Write-Host "[OK] Found: $($backupFile.name) ($fileSizeMB MB)" -ForegroundColor Green
    Write-Host "    Modified: $($backupFile.modifiedTime)" -ForegroundColor Gray
    Write-Host ""
    
    # Download archive
    Write-Host "[*] Downloading archive..." -ForegroundColor Cyan
    $tempArchive = Join-Path $env:TEMP $backupFile.name
    
    $startDownload = Get-Date
    $success = Download-File -FileId $backupFile.id -DestPath $tempArchive -Token $token
    $downloadTime = ((Get-Date) - $startDownload).TotalSeconds
    
    if (-not $success) {
        Write-Host "[ERROR] Download failed" -ForegroundColor Red
        exit 1
    }
    
    $downloadSpeed = if ($downloadTime -gt 0) { [math]::Round($fileSizeMB / $downloadTime, 2) } else { 0 }
    Write-Host "[OK] Downloaded in $([math]::Round($downloadTime, 1)) sec ($downloadSpeed MB/s)" -ForegroundColor Green
    Write-Host ""
    
    # Extract archive
    Write-Host "[*] Extracting archive..." -ForegroundColor Cyan
    $startExtract = Get-Date
    
    try {
        # Create destination if doesn't exist
        if (-not (Test-Path $DestinationPath)) {
            New-Item -ItemType Directory -Path $DestinationPath -Force | Out-Null
        }
        
        Initialize-CompressionSupport
        [System.IO.Compression.ZipFile]::ExtractToDirectory($tempArchive, $DestinationPath, $true)
        
        $extractTime = ((Get-Date) - $startExtract).TotalSeconds
        
        # Count extracted files
        $extractedFiles = (Get-ChildItem -Path $DestinationPath -Recurse -File).Count
        
        Write-Host "[OK] Extracted $extractedFiles files in $([math]::Round($extractTime, 1)) sec" -ForegroundColor Green
        Write-Host ""
        
        # Cleanup
        Remove-Item $tempArchive -Force -ErrorAction SilentlyContinue
        
        Write-Host "================================================================" -ForegroundColor Green
        Write-Host "                    Pull Complete                               " -ForegroundColor Green
        Write-Host "================================================================" -ForegroundColor Green
        Write-Host ""
        Write-Host "  Status:           " -NoNewline
        Write-Host "SUCCESS" -ForegroundColor Green
        Write-Host ""
        Write-Host "  Archive:          $($backupFile.name)"
        Write-Host "  Size:             $fileSizeMB MB"
        Write-Host "  Files:            $extractedFiles"
        Write-Host "  Download Time:    $([math]::Round($downloadTime, 1)) sec"
        Write-Host "  Extract Time:     $([math]::Round($extractTime, 1)) sec"
        Write-Host "  Total Time:       $([math]::Round($downloadTime + $extractTime, 1)) sec"
        Write-Host "  Location:         $DestinationPath" -ForegroundColor Cyan
        Write-Host ""
        Write-Host "================================================================" -ForegroundColor Green
        Write-Host ""
        exit 0
        
    } catch {
        Write-Host "[ERROR] Failed to extract: $($_.Exception.Message)" -ForegroundColor Red
        Remove-Item $tempArchive -Force -ErrorAction SilentlyContinue
        exit 1
    }
}

# Upload files
if ($SourcePath -and $FolderId) {
    Write-Title "STEM Cloud Backup"
    Write-Note "Destination: Google Drive"
    
    # Resolve full source path
    $SourcePath = Resolve-Path $SourcePath
    
    # Get all files to scan
    Write-Phase 1 "Scan project"
    Write-Host "  Scanning files..." -ForegroundColor Gray
    $files = Get-ChildItem -Path $SourcePath -Recurse -File -ErrorAction SilentlyContinue | Where-Object {
        # Exclude .stem directory
        $_.FullName -notmatch '[\\/]\.stem[\\/]' -and $_.FullName -notlike '*\.stem'
    }
    
    # Load .stemignore patterns if exists
    $stemignorePath = Join-Path $SourcePath ".stemignore"
    $ignorePatterns = @()
    
    if (Test-Path $stemignorePath) {
        Write-Host "  Loading .stemignore..." -ForegroundColor Gray
        $ignorePatterns = Get-Content $stemignorePath | Where-Object {
            $_ -notmatch '^\s*#' -and $_ -notmatch '^\s*$'
        } | ForEach-Object { $_.Trim() }
        
        if ($ignorePatterns.Count -gt 0) {
            Write-Host "  Applying $($ignorePatterns.Count) ignore patterns..." -ForegroundColor Gray
            
            # Filter files based on ignore patterns
            $files = $files | Where-Object {
                $file = $_
                $relativePath = $file.FullName.Substring($SourcePath.Length).TrimStart('\', '/')
                $shouldInclude = $true
                
                foreach ($pattern in $ignorePatterns) {
                    # Directory pattern (ends with /)
                    if ($pattern.EndsWith('/')) {
                        $dirPattern = $pattern.TrimEnd('/')
                        if ($relativePath -like "*$dirPattern*") {
                            $shouldInclude = $false
                            break
                        }
                    }
                    # Extension pattern (starts with *)
                    elseif ($pattern.StartsWith('*')) {
                        if ($file.Name -like $pattern) {
                            $shouldInclude = $false
                            break
                        }
                    }
                    # Exact filename pattern
                    else {
                        if ($relativePath -like "*$pattern*") {
                            $shouldInclude = $false
                            break
                        }
                    }
                }
                
                $shouldInclude
            }
        }
    }
    
    $total = $files.Count
    
    if ($total -eq 0) {
        Write-Host ""
        Write-Host "[WARNING] No files found to upload" -ForegroundColor Yellow
        Write-Host ""
        exit 0
    }
    
    # Calculate total size
    $totalSize = ($files | Measure-Object -Property Length -Sum).Sum
    $sizeMB = [math]::Round($totalSize / 1MB, 2)
    
    Write-Host "  OK  $total files selected ($sizeMB MB)" -ForegroundColor Green
    
    # Create temporary archive
    Write-Phase 2 "Create archive"
    Write-Host "  Building compressed backup..." -ForegroundColor Gray
    $timestamp = Get-Date -Format 'yyyyMMdd_HHmmss'
    $archiveName = "stem_backup_$timestamp.zip"
    $tempArchive = Join-Path $env:TEMP $archiveName
    
    # Remove old temp archive if exists
    if (Test-Path $tempArchive) {
        Remove-Item $tempArchive -Force
    }
    
    $startCompress = Get-Date
    
    try {
        # Create zip archive with compression
        Initialize-CompressionSupport
        $compressionLevel = [System.IO.Compression.CompressionLevel]::Optimal
        
        # Create archive
        $archive = [System.IO.Compression.ZipFile]::Open($tempArchive, [System.IO.Compression.ZipArchiveMode]::Create)
        
        $processed = 0
        $lastShownPercent = -1
        foreach ($file in $files) {
            $processed++
            $percent = [int](($processed / $total) * 100)
            
            # Calculate relative path to maintain directory structure
            $relativePath = $file.FullName.Substring($SourcePath.Length).TrimStart('\', '/')
            
            # Add file to archive
            $entry = $archive.CreateEntry($relativePath, $compressionLevel)
            $entryStream = $entry.Open()
            $fileStream = [System.IO.File]::OpenRead($file.FullName)
            $fileStream.CopyTo($entryStream)
            $fileStream.Close()
            $entryStream.Close()
            
            # Show cleaner progress at 5% intervals.
            if (([int]($percent / 5) -gt [int]($lastShownPercent / 5)) -or ($percent -eq 100 -and $lastShownPercent -ne 100)) {
                Write-Host ("`r  Progress: {0,3}% ({1}/{2} files)" -f $percent, $processed, $total) -NoNewline -ForegroundColor DarkGray
                $lastShownPercent = $percent
            }
        }

        Write-Host ""
        
        $archive.Dispose()
        
        $compressTime = ((Get-Date) - $startCompress).TotalSeconds
        $archiveSize = (Get-Item $tempArchive).Length
        $archiveMB = [math]::Round($archiveSize / 1MB, 2)
        $ratio = [math]::Round((1 - ($archiveSize / $totalSize)) * 100, 1)
        
        Write-Host "  OK  Archive ready" -ForegroundColor Green
        Write-Note "Compressed size: $archiveMB MB ($ratio% reduction)"
        Write-Note "Compression time: $([math]::Round($compressTime, 1)) sec"
        
    } catch {
        Write-Host "[ERROR] Failed to create archive: $($_.Exception.Message)" -ForegroundColor Red
        exit 1
    }
    
    Write-Phase 3 "Upload to Google Drive"
    Write-Host "  Uploading $archiveName ($archiveMB MB)..." -ForegroundColor Gray
    Write-Note "Large files can take a few minutes depending on your connection."
    
    $startUpload = Get-Date
    $result = Upload-File -FilePath $tempArchive -ParentFolderId $FolderId -Token $token
    $uploadTime = ((Get-Date) - $startUpload).TotalSeconds
    
    # Cleanup temp archive
    Remove-Item $tempArchive -Force -ErrorAction SilentlyContinue
    
    if ($result) {
        Cleanup-OldBackups -FolderId $FolderId -Token $token -KeepCount $KeepBackups

        $uploadSpeed = if ($uploadTime -gt 0) { [math]::Round($archiveMB / $uploadTime, 2) } else { 0 }
        
        Write-Title "Backup Complete"
        Write-Host "  Status: SUCCESS" -ForegroundColor Green
        Write-Host ""
        Write-Host "  Files:           $total"
        Write-Host "  Original size:   $sizeMB MB"
        Write-Host "  Archive size:    $archiveMB MB"
        Write-Host "  Size reduction:  $ratio%"
        Write-Host "  Compress time:   $([math]::Round($compressTime, 1)) sec"
        Write-Host "  Upload time:     $([math]::Round($uploadTime, 1)) sec"
        Write-Host "  Upload speed:    $uploadSpeed MB/s"
        Write-Host "  Total time:      $([math]::Round($compressTime + $uploadTime, 1)) sec"
        Write-Host ""
        Write-Host "  Archive:         $archiveName" -ForegroundColor Cyan
        if ($result.webViewLink) {
            Write-Host "  File URL:        $($result.webViewLink)" -ForegroundColor Cyan
        }
        Write-Host "  Folder URL:      https://drive.google.com/drive/folders/$FolderId" -ForegroundColor Cyan
        Write-Host ""
        exit 0
    } else {
        Write-Host ""
        if ($script:LastUploadAuthError) {
            Write-Host "[ERROR] Upload failed due to authorization" -ForegroundColor Red
            exit 2
        }

        Write-Host "[ERROR] Upload failed" -ForegroundColor Red
        Write-Host ""
        exit 1
    }
}

Write-Host ""
Write-Host "Usage:" -ForegroundColor Yellow
Write-Host "  Setup:  .\gdrive_upload.ps1 -SetupAuth"
Write-Host "  Upload: .\gdrive_upload.ps1 -SourcePath <path> -FolderId <id>"
Write-Host "          .\gdrive_upload.ps1 -SourcePath <path> -FolderId <id> -KeepBackups <N>"
Write-Host ""
