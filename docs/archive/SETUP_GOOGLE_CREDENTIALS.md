# Setup Google Drive Credentials - 10 Minutes

## Why This Is Needed

Google requires ALL apps to register with Google Cloud Console before they can upload to Drive. This is a one-time setup that takes about 10 minutes.

## Step-by-Step Guide

### Step 1: Create Google Cloud Project (2 minutes)

1. Go to: https://console.cloud.google.com/
2. Click **"New Project"** (top right)
3. Project name: `STEM VCS`
4. Click **"Create"**
5. Wait for project to be created (~10 seconds)

### Step 2: Enable Google Drive API (1 minute)

1. Make sure your new project is selected (top left dropdown)
2. Go to: **"APIs & Services"** > **"Library"** (left menu)
3. Search for: `Google Drive API`
4. Click on **"Google Drive API"**
5. Click **"Enable"**

### Step 3: Configure OAuth Consent Screen (3 minutes)

1. Go to: **"APIs & Services"** > **"OAuth consent screen"** (left menu)
2. Select: **"External"**
3. Click **"Create"**

Fill in the form:
- **App name**: `STEM VCS`
- **User support email**: Your email (select from dropdown)
- **Developer contact**: Your email

4. Click **"Save and Continue"**
5. On "Scopes" page: Click **"Add or Remove Scopes"**
   - Search for: `drive.file`
   - Check: `../auth/drive.file` (See, edit, create, and delete only files created with this app)
6. Click **"Update"** then **"Save and Continue"**
7. On "Test users" page: Click **"Add Users"**
   - Enter your Gmail address: `your-email@example.com`
   - Click **"Add"**
8. Click **"Save and Continue"**
9. Review and click **"Back to Dashboard"**

### Step 4: Create OAuth Client ID (2 minutes)

1. Go to: **"APIs & Services"** > **"Credentials"** (left menu)
2. Click **"Create Credentials"** > **"OAuth client ID"**
3. Application type: Select **"Desktop app"**
4. Name: `STEM Desktop Client`
5. Click **"Create"**

### Step 5: Download Credentials (1 minute)

1. You'll see a popup with your credentials
2. Click **"Download JSON"** (or copy the Client ID and Client Secret)
3. Open the downloaded JSON file with Notepad

You'll see something like:
```json
{
  "installed": {
    "client_id": "532668927934-abc123xyz.apps.googleusercontent.com",
    "client_secret": "YOUR_CLIENT_SECRET",
    ...
  }
}
```

### Step 6: Configure STEM (2 minutes)

1. Set the OAuth client ID and secret in your process environment:
```powershell
$env:STEM_GDRIVE_CLIENT_ID = "YOUR_CLIENT_ID.apps.googleusercontent.com"
$env:STEM_GDRIVE_CLIENT_SECRET = "YOUR_CLIENT_SECRET"
```
Use your own OAuth credentials. Do not commit credentials or store them in this script.

### Step 7: Authorize STEM (2 minutes)

1. Open PowerShell
2. Run:
```powershell
cd C:\path\to\STEM_VCS
powershell -ExecutionPolicy Bypass -File .\gdrive_upload.ps1 -SetupAuth
```

3. Browser opens → Sign in with `your-email@example.com`
4. Click **"Allow"**
5. Google shows you a code
6. Copy the code
7. Paste it in PowerShell
8. Press Enter

Done! Authorization saved!

### Step 8: Test Upload

```cmd
cd E:\STEM_VCS\previous_Versions\version_0
stem push
```

Your files will upload to Google Drive!

---

## Screenshots of What You'll See

### Google Cloud Console
1. "New Project" button → Create "STEM VCS"
2. "Enable APIs" → Search "Google Drive API" → Enable
3. "OAuth consent screen" → External → Fill form
4. "Credentials" → "Create OAuth client ID" → Desktop app
5. Download JSON

### Authorization Screen
When you run SetupAuth:
- Browser opens to Google sign-in
- Shows: "STEM VCS wants to access your Google Drive"
- Click "Allow"
- Shows: "Please copy this code, switch to your application and paste it there:"
- Copy the code → Paste in PowerShell → Done!

---

## Important Notes

- **One-time setup**: You only do this once per computer
- **Your credentials**: Keep client_secret private (don't share)
- **Test user**: Make sure you added `your-email@example.com` as a test user
- **App not verified**: Google will show "App isn't verified" - click "Advanced" → "Go to STEM VCS (unsafe)" - this is normal for personal projects
- **Revoke access**: You can revoke anytime at https://myaccount.google.com/permissions

---

## Troubleshooting

### "Error 401: invalid_client"
- You need to replace the placeholder credentials with your own
- Follow Step 5 and Step 6 above

### "Access blocked: This app's request is invalid"
- Make sure you enabled Google Drive API (Step 2)
- Make sure you added yourself as a test user (Step 3)

### "App isn't verified"
- This is normal for personal projects
- Click "Advanced" → "Go to STEM VCS (unsafe)"
- This is safe - it's YOUR app

### "The OAuth client was not found"
- The client_id in gdrive_upload.ps1 doesn't match your Google Cloud project
- Make sure you copied the correct client_id from the JSON

---

## Why Can't STEM Provide Credentials?

Google's OAuth policy requires each developer to create their own credentials. This is for security and prevents abuse. If STEM provided shared credentials, anyone could misuse them.

By creating your own credentials:
- ✓ You control who has access
- ✓ You can revoke access anytime
- ✓ More secure
- ✓ Follows Google's best practices

---

## Summary

**Time needed**: 10 minutes (one-time)

**Steps**:
1. Create Google Cloud project
2. Enable Drive API
3. Configure OAuth consent
4. Create credentials
5. Download JSON
6. Edit gdrive_upload.ps1 with your credentials
7. Run SetupAuth
8. Done!

After this, `stem push` works forever!

---

## Quick Checklist

- [ ] Created Google Cloud project "STEM VCS"
- [ ] Enabled Google Drive API
- [ ] Configured OAuth consent screen (External)
- [ ] Added your-email@example.com as test user
- [ ] Created OAuth Desktop app credentials
- [ ] Downloaded JSON file
- [ ] Edited gdrive_upload.ps1 with client_id and client_secret
- [ ] Ran: `powershell -ExecutionPolicy Bypass -File .\gdrive_upload.ps1 -SetupAuth`
- [ ] Authorized STEM in browser
- [ ] Copied and pasted code
- [ ] Tested with: `stem push`

All done? You can now use `stem push` anytime!

---

Copyright 2026 STEM VCS
