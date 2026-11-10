# Security Policy

## Reporting a Vulnerability

Please do not disclose exploitable vulnerabilities in public issues. Use the private vulnerability-reporting feature of the project's hosting platform, or contact the maintainers through a private channel listed there.

Include the affected version, impact, and steps to reproduce. Do not include real credentials, access tokens, or private user data in reports.

## Credential Handling

Keep OAuth client secrets, refresh tokens, and credential files out of source control. The Google Drive integration reads its OAuth client ID and secret from `STEM_GDRIVE_CLIENT_ID` and `STEM_GDRIVE_CLIENT_SECRET` environment variables. Revoke and replace any credential that has been committed or shared.