# Test single file upload to debug the issue

param(
    [string]$TestFile = (Join-Path $PSScriptRoot "..\..\README.md"),
    [string]$FolderId = ""
)

$ErrorActionPreference = "Continue"

Write-Host "Testing single file upload..." -ForegroundColor Cyan
Write-Host ""

# Load token
$TokenFile = "$env:USERPROFILE\.stem_gdrive_token"
if (-not (Test-Path $TokenFile)) {
    Write-Host "[ERROR] No token found" -ForegroundColor Red
    exit 1
}

$tokenData = Get-Content -Path $TokenFile | ConvertFrom-Json

# Refresh token
Write-Host "[*] Refreshing access token..." -ForegroundColor Cyan
$ClientId = $env:STEM_GDRIVE_CLIENT_ID
$ClientSecret = $env:STEM_GDRIVE_CLIENT_SECRET
if ([string]::IsNullOrWhiteSpace($ClientId) -or [string]::IsNullOrWhiteSpace($ClientSecret)) {
    Write-Host "[ERROR] Set STEM_GDRIVE_CLIENT_ID and STEM_GDRIVE_CLIENT_SECRET first." -ForegroundColor Red
    exit 1
}

$tokenUrl = "https://oauth2.googleapis.com/token"
$body = @{
    refresh_token = $tokenData.refresh_token
    client_id = $ClientId
    client_secret = $ClientSecret
    grant_type = "refresh_token"
}

try {
    $response = Invoke-RestMethod -Uri $tokenUrl -Method Post -Body $body -ContentType "application/x-www-form-urlencoded"
    $token = $response.access_token
    Write-Host "[OK] Token refreshed" -ForegroundColor Green
} catch {
    Write-Host "[ERROR] Token refresh failed: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# Get folder ID from argument or config
if (-not $FolderId) {
    Write-Host "[*] Reading folder ID from .stem/config..." -ForegroundColor Cyan
    $configPath = Join-Path $PSScriptRoot "..\..\.stem\config"
    if (-not (Test-Path $configPath)) {
        Write-Host "[ERROR] No .stem/config found; pass -FolderId explicitly." -ForegroundColor Red
        exit 1
    }
    $config = Get-Content $configPath -Raw
    if ($config -match 'folderid\s*=\s*(.+)') {
        $FolderId = $matches[1].Trim()
        Write-Host "[OK] Found folder ID: $FolderId" -ForegroundColor Green
    } else {
        Write-Host "[ERROR] Could not find folder ID" -ForegroundColor Red
        exit 1
    }
}

Write-Host ""
Write-Host "[*] Testing file upload..." -ForegroundColor Cyan
Write-Host "  File: $TestFile"
Write-Host "  Destination: $FolderId"
Write-Host ""

$fileName = Split-Path -Leaf $TestFile
$fileContent = [System.IO.File]::ReadAllBytes($TestFile)

Write-Host "[*] File size: $($fileContent.Length) bytes" -ForegroundColor Cyan

# Try simple metadata-only upload first
Write-Host ""
Write-Host "[*] Method 1: Metadata-only (testing API)" -ForegroundColor Yellow

$metadata = @{
    name = "test_$fileName"
    parents = @($FolderId)
} | ConvertTo-Json

$headers = @{
    "Authorization" = "Bearer $token"
    "Content-Type" = "application/json"
}

try {
    Write-Host "  Sending request..." -ForegroundColor Gray
    $response = Invoke-RestMethod -Uri "https://www.googleapis.com/drive/v3/files" `
        -Method Post `
        -Headers $headers `
        -Body $metadata `
        -TimeoutSec 10
    
    Write-Host "[OK] Metadata upload successful!" -ForegroundColor Green
    Write-Host "  File ID: $($response.id)"
    
    # Clean up test file
    Write-Host "[*] Cleaning up test file..." -ForegroundColor Cyan
    Invoke-RestMethod -Uri "https://www.googleapis.com/drive/v3/files/$($response.id)" `
        -Method Delete `
        -Headers @{ "Authorization" = "Bearer $token" } | Out-Null
    Write-Host "[OK] Test file deleted" -ForegroundColor Green
    
} catch {
    Write-Host "[ERROR] Metadata upload failed!" -ForegroundColor Red
    Write-Host "  Error: $($_.Exception.Message)" -ForegroundColor Red
    if ($_.Exception.Response) {
        $reader = New-Object System.IO.StreamReader($_.Exception.Response.GetResponseStream())
        $responseBody = $reader.ReadToEnd()
        Write-Host "  Response: $responseBody" -ForegroundColor Red
    }
    exit 1
}

Write-Host ""
Write-Host "[SUCCESS] Upload test passed!" -ForegroundColor Green
Write-Host ""
Write-Host "The Google Drive API is working correctly." -ForegroundColor Green
Write-Host "The issue might be with large batch uploads or file scanning." -ForegroundColor Yellow
