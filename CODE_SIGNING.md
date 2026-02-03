# Code Signing Guide for DiabloLikeGame

## Overview

This document provides guidance on code signing for DiabloLikeGame.exe to eliminate Windows SmartScreen warnings in production environments.

## Current State

The project now includes:
- **Application Manifest** (`app.manifest`) - Defines execution level and compatibility
- **Version Resources** (`Resource.rc`, `resource.h`) - Embeds publisher and version information
- **Documentation** - README updated with SmartScreen information

These changes help Windows recognize the application but don't eliminate SmartScreen warnings completely. A digital code signing certificate is required for production use.

## What is Code Signing?

Code signing digitally signs your executable with a trusted certificate, proving:
- The publisher's identity is verified
- The code hasn't been tampered with since signing
- The application comes from a legitimate source

## Obtaining a Code Signing Certificate

### Step 1: Purchase a Certificate

**Recommended Certificate Authorities:**
- **DigiCert** - $474/year for EV Code Signing
- **Sectigo (formerly Comodo)** - $474/year for EV Code Signing  
- **GlobalSign** - $599/year for EV Code Signing
- **Certum** - Starting at $149/year for standard OV certificates

**Certificate Types:**
1. **Standard (OV) Code Signing** (~$150-300/year)
   - Basic identity verification
   - SmartScreen reputation starts at zero
   - Must build reputation over time

2. **Extended Validation (EV) Code Signing** (~$400-600/year)
   - Rigorous identity verification
   - Instant SmartScreen reputation
   - Requires hardware token
   - **Recommended for immediate trust**

### Step 2: Complete Identity Verification

Requirements typically include:
- Business registration documents
- Government-issued ID
- Phone verification
- Physical address verification
- DUNS number (for EV certificates)

This process can take 1-7 business days.

### Step 3: Receive Certificate

**For Standard Certificates:**
- Download certificate file (.pfx or .p12)
- Store securely with password

**For EV Certificates:**
- Receive hardware USB token
- Certificate is stored on the token (cannot be exported)

## Signing the Executable

### Option 1: Manual Signing with signtool

1. **Install Windows SDK** (includes signtool.exe)
   ```powershell
   # Usually located at:
   # C:\Program Files (x86)\Windows Kits\10\bin\<version>\x64\signtool.exe
   ```

2. **Sign the executable**
   ```powershell
   # For .pfx file
   signtool sign /f "path\to\certificate.pfx" /p "password" /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 "DiabloLikeGame.exe"
   
   # For certificate in Windows Certificate Store
   signtool sign /n "Certificate Subject Name" /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 "DiabloLikeGame.exe"
   
   # For EV certificate on USB token
   signtool sign /sha1 "certificate thumbprint" /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 "DiabloLikeGame.exe"
   ```

3. **Verify signature**
   ```powershell
   signtool verify /pa "DiabloLikeGame.exe"
   ```

### Option 2: Automated Signing in Build Pipeline

Add post-build event to `DiabloLikeGame.vcxproj`:

```xml
<PropertyGroup>
  <PostBuildEvent>
    <Command>
      if "$(Configuration)"=="Release" (
        "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\signtool.exe" sign /f "$(SolutionDir)certificates\codesign.pfx" /p "$(CodeSignPassword)" /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 "$(TargetPath)"
      )
    </Command>
  </PostBuildEvent>
</PropertyGroup>
```

**Important:** Never commit certificate files or passwords to source control!

### Option 3: GitHub Actions Signing

For automated releases, use GitHub Actions with certificates stored as secrets:

```yaml
- name: Sign Executable
  env:
    CERTIFICATE_BASE64: ${{ secrets.CERTIFICATE_BASE64 }}
    CERTIFICATE_PASSWORD: ${{ secrets.CERTIFICATE_PASSWORD }}
  run: |
    # Decode certificate from base64
    $certBytes = [Convert]::FromBase64String($env:CERTIFICATE_BASE64)
    $certPath = "$env:TEMP\cert.pfx"
    [IO.File]::WriteAllBytes($certPath, $certBytes)
    
    # Sign the executable
    & "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\signtool.exe" sign `
      /f $certPath `
      /p $env:CERTIFICATE_PASSWORD `
      /fd SHA256 `
      /tr http://timestamp.digicert.com `
      /td SHA256 `
      "bin/x64/Release/DiabloLikeGame.exe"
    
    # Clean up
    Remove-Item $certPath
  shell: pwsh
```

## Best Practices

### Security
- **Never commit certificates** to version control
- Use environment variables or secure vaults for passwords
- Restrict access to certificates to necessary personnel only
- Use hardware tokens (EV certificates) when possible
- Revoke certificates immediately if compromised

### Timestamping
- **Always use timestamping** (`/tr` flag in signtool)
- Ensures signature remains valid after certificate expires
- Use reliable timestamp servers:
  - DigiCert: `http://timestamp.digicert.com`
  - GlobalSign: `http://timestamp.globalsign.com/tsa/r6advanced1`
  - Sectigo: `http://timestamp.sectigo.com`

### Testing
- Test signed executables on clean Windows machines
- Verify SmartScreen doesn't show warnings
- Check certificate details in file properties
- Validate signature: Right-click exe → Properties → Digital Signatures

### Reputation Building
- Even with valid signatures, new certificates start with zero reputation
- Build reputation by:
  - Having users download and run your software
  - Maintaining consistent publisher identity
  - Avoiding suspicious behavior patterns
- EV certificates bypass reputation requirement

## Alternatives for Open Source Projects

### Free Options
1. **Let users bypass SmartScreen manually** (current approach)
   - Document the process clearly
   - Acceptable for educational/open-source projects
   
2. **Build reputation over time**
   - As more users run your software, SmartScreen warnings decrease
   - Can take months to build sufficient reputation

3. **Microsoft Store distribution**
   - Apps distributed through Microsoft Store are trusted
   - Requires Microsoft Store developer account ($19 one-time fee)

### Community Signing Services
Some communities offer code signing services:
- **SignPath Foundation** - Free for open source projects
  - Visit: https://signpath.org
  - Application process required
  - Suitable for established open source projects

## Cost-Benefit Analysis

**For Personal/Educational Projects:**
- Current solution (manifest + version info) is sufficient
- Users can bypass SmartScreen with "Run anyway"
- Code signing certificate cost ($150-600/year) may not be justified

**For Commercial/Professional Distribution:**
- Code signing is strongly recommended
- Improves user trust and reduces support requests
- EV certificate provides immediate trust
- Cost is tax-deductible business expense

## Troubleshooting

### "The file is not signed" error
- Ensure signtool command completed successfully
- Check for error messages during signing
- Verify certificate is valid and not expired

### SmartScreen still shows warnings after signing
- Check certificate type (OV vs EV)
- Verify timestamp was applied correctly
- For OV certificates, reputation must be built over time
- Test on different Windows machines

### Certificate errors
- Ensure certificate chain is complete
- Install intermediate certificates if needed
- Verify certificate is trusted by Windows

## References

- [Microsoft Code Signing Best Practices](https://docs.microsoft.com/en-us/windows-hardware/drivers/dashboard/code-signing-best-practices)
- [SignTool Documentation](https://docs.microsoft.com/en-us/windows/win32/seccrypto/signtool)
- [Windows Authenticode Signing](https://docs.microsoft.com/en-us/windows-hardware/drivers/install/authenticode)

## Summary

While code signing is the ultimate solution for SmartScreen warnings, the current implementation with application manifest and version resources provides:
- Clear application identity
- Proper execution level declaration
- Version information in file properties
- Good foundation for future code signing

For production distribution, budget for an EV code signing certificate to provide the best user experience.
