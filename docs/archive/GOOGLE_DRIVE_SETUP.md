# Google Drive Setup for STEM VCS

## Overview

STEM VCS can upload your repositories directly to Google Drive for cloud backup. This guide walks you through the one-time setup process.

## Quick Start

If you just want to get started quickly:

1. **Use Google Drive Desktop App** (easiest option)
   - Download from: https://www.google.com/drive/download/
   - Put your project folder inside Google Drive folder
   - Files sync automatically, no setup needed!

2. **Use Direct Upload** (requires Google API setup)
   - Follow the steps below to enable Google Drive API
   - More complex but gives you full control

## Option 1: Google Drive Desktop App (Recommended for Beginners)

### Steps:

1. **Install Google Drive Desktop**
   ```
   https://www.google.com/drive/download/
   ```

2. **Sign in** with your Google account

3. **Move your project to Google Drive folder**
   ```cmd
   move E:\STEM_VCS\previous_Versions\version_0 "C:\Users\YourName\Google Drive\XylusOS"
   ```

4. **Use STEM normally**
   ```cmd
   cd "C:\Users\YourName\Google Drive\XylusOS"
   stem init
   stem add .
   stem commit -m "Initial commit"
   ```
   
   Files automatically sync to Google Drive!

### Pros:
- ✓ No API setup required
- ✓ Automatic syncing
- ✓ Works immediately
- ✓ Simple to use

### Cons:
- ✗ Requires Google Drive Desktop app
- ✗ Uses local disk space
- ✗ Not integrated into `stem push` command

---

## Option 2: Direct Google Drive API Upload

This option integrates Google Drive upload directly into the `stem push` command.

### Prerequisites:
- Google account
- Google Cloud Console access
- PowerShell (Windows) or pwsh (Linux/Mac)

### Step 1: Create Google Cloud Project

1. Go to https://console.cloud.google.com/
2. Click **"New Project"**
3. Name it: **"STEM VCS"**
4. Click **"Create"**

### Step 2: Enable Google Drive API

1. In your project, go to **"APIs & Services"** > **"Library"**
2. Search for **"Google Drive API"**
3. Click on it and press **"Enable"**

### Step 3: Create OAuth 2.0 Credentials

1. Go to **"APIs & Services"** > **"Credentials"**
2. Click **"Create Credentials"** > **"OAuth client ID"**
3. If prompted, configure the consent screen:
   - User Type: **External**
   - App name: **STEM VCS**
   - User support email: Your email
   - Developer contact: Your email
   - Scopes: Add **`../auth/drive.file`**
   - Test users: Add your Gmail address
4. Create OAuth client ID:
   - Application type: **Desktop app**
   - Name: **STEM Desktop Client**
5. Click **"Create"**
6. **IMPORTANT**: Download the credentials JSON file

### Step 4: Configure STEM VCS

1. Open the downloaded credentials JSON file
2. Find these values:
   - `client_id`
   - `client_secret`

3. Edit `gdrive_upload.ps1` in your STEM installation directory:
   ```powershell
   # Find these lines and replace with your values:
   $ClientId = "YOUR_CLIENT_ID.apps.googleusercontent.com"
   $ClientSecret = "YOUR_CLIENT_SECRET"
   ```

4. Save the file

### Step 5: Authorize STEM VCS

Run the authorization setup:

```powershell
cd C:\path\to\STEM_VCS
.\gdrive_upload.ps1 -SetupAuth
```

This will:
1. Open your browser
2. Ask you to sign in to Google
3. Show a consent screen
4. Ask you to authorize STEM VCS
5. Give you an authorization code
6. Save the credentials to `.stem/gdrive_token`

### Step 6: Test Upload

```cmd
cd E:\STEM_VCS\previous_Versions\version_0
stem init --cloud "https://drive.google.com/drive/folders/YOUR_FOLDER_ID"
stem add .
stem commit -m "Initial commit"
stem push
```

The `stem push` command will now upload directly to Google Drive!

---

## Troubleshooting

### "Authorization failed"
- Re-run: `.\gdrive_upload.ps1 -SetupAuth`
- Make sure you added yourself as a test user in Google Cloud Console
- Check that the Google Drive API is enabled

### "Client ID not found"
- Make sure you edited `gdrive_upload.ps1` with your credentials
- Double-check the client_id and client_secret are correct

### "Access denied"
- Add yourself as a test user in Google Cloud Console:
  - Go to "OAuth consent screen"
  - Scroll to "Test users"
  - Click "Add Users"
  - Enter your Gmail address

### "Folder ID not found"
- Check your Google Drive URL
- Make sure it's a folder, not a file
- Format: `https://drive.google.com/drive/folders/FOLDER_ID`

### "PowerShell script not found"
- Make sure STEM is properly installed
- Script should be at: `C:\path\to\STEM_VCS\gdrive_upload.ps1`
- Run `stem push` from a repository directory, not the STEM installation directory

---

## How It Works

When you run `stem push`:

1. STEM reads `.stem/config` to get the Google Drive folder ID
2. STEM calls `gdrive_upload.ps1` with the folder ID
3. PowerShell script loads your saved authorization token
4. Script uploads all files from `.stem/` directory to Google Drive
5. Creates a timestamped backup folder in your Drive
6. Shows progress for each file uploaded

Your repository data is stored in Google Drive in a folder structure like:
```
Your Google Drive Folder/
└── stem_backup_20260802_143025/
    ├── objects/
    │   ├── ab/
    │   │   └── 1234567890abcdef...
    │   └── cd/
    │       └── 1234567890abcdef...
    ├── refs/
    │   └── heads/
    │       └── main
    ├── HEAD
    ├── config
    └── index
```

---

## Security Notes

- Your OAuth token is stored in `.stem/gdrive_token` (keep this private!)
- Add to `.gitignore`: `.stem/gdrive_token`
- Token gives access only to files created by STEM
- You can revoke access anytime from: https://myaccount.google.com/permissions
- STEM never sees your Google password

---

## Comparison: Desktop App vs Direct API

| Feature | Desktop App | Direct API |
|---------|-------------|------------|
| Setup Time | 5 minutes | 30-60 minutes |
| Technical Level | Beginner | Advanced |
| Disk Space | Uses local space | No extra space |
| Auto Sync | Yes | Manual (stem push) |
| Integration | External app | Built into STEM |
| Internet | Background sync | On-demand upload |
| Control | Automatic | Explicit commands |

**Recommendation**: Start with Google Drive Desktop app. Switch to Direct API later if needed.

---

## Next Steps

After setup:
- ✓ Your commits are backed up to Google Drive
- ✓ Others can clone using your Google Drive URL
- ✓ View files at: https://drive.google.com/drive/folders/YOUR_FOLDER_ID

Commands:
```cmd
stem add .                    # Stage files
stem commit -m "message"      # Create commit
stem push                     # Upload to Google Drive
stem status                   # View repository status
```

---

## Support

Having issues? Check:
1. PUSH_STATUS.txt - Current implementation status
2. README.txt - General STEM documentation
3. This file - Setup instructions

For Google Cloud Console help:
- https://console.cloud.google.com/
- https://developers.google.com/drive/api/guides/about-sdk

---

Copyright 2026 STEM VCS
