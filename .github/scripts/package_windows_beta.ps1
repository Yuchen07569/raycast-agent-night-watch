param(
  [Parameter(Mandatory = $true)]
  [string]$BuildDirectory,

  [Parameter(Mandatory = $true)]
  [string]$OutputDirectory
)

$ErrorActionPreference = "Stop"
$version = "0.2.0-beta.1"
$executable = Join-Path $BuildDirectory "AgentNightWatch.exe"
if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) {
  throw "AgentNightWatch.exe was not found in $BuildDirectory"
}

New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$archiveName = "AgentNightWatch-$version-windows-x64.zip"
$archivePath = Join-Path $OutputDirectory $archiveName
$checksumPath = "$archivePath.sha256"
$sbomPath = Join-Path $OutputDirectory "AgentNightWatch-$version.spdx.json"

Compress-Archive -LiteralPath $executable -DestinationPath $archivePath -Force
$archiveHash = (Get-FileHash -LiteralPath $archivePath -Algorithm SHA256).Hash.ToLowerInvariant()
Set-Content -LiteralPath $checksumPath -Encoding ascii -NoNewline -Value "$archiveHash  $archiveName`n"

$executableHash = (Get-FileHash -LiteralPath $executable -Algorithm SHA256).Hash.ToLowerInvariant()
$created = [DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ")
$namespace = "https://github.com/Yuchen07569/raycast-agent-night-watch/releases/tag/windows-v$version"
$sbom = [ordered]@{
  spdxVersion = "SPDX-2.3"
  dataLicense = "CC0-1.0"
  SPDXID = "SPDXRef-DOCUMENT"
  name = "AgentNightWatch-$version-windows-x64"
  documentNamespace = $namespace
  creationInfo = [ordered]@{
    created = $created
    creators = @("Tool: raycast-agent-night-watch GitHub Actions", "Organization: Yuchen07569")
  }
  packages = @(
    [ordered]@{
      name = "Agent Night Watch"
      SPDXID = "SPDXRef-Package-AgentNightWatch"
      versionInfo = $version
      downloadLocation = "NOASSERTION"
      filesAnalyzed = $true
      licenseConcluded = "MIT"
      licenseDeclared = "MIT"
      copyrightText = "Copyright (c) 2026 Yuchen07569"
    }
  )
  files = @(
    [ordered]@{
      fileName = "./AgentNightWatch.exe"
      SPDXID = "SPDXRef-File-AgentNightWatchExe"
      checksums = @(
        [ordered]@{
          algorithm = "SHA256"
          checksumValue = $executableHash
        }
      )
      licenseConcluded = "MIT"
      copyrightText = "Copyright (c) 2026 Yuchen07569"
    }
  )
  relationships = @(
    [ordered]@{
      spdxElementId = "SPDXRef-DOCUMENT"
      relationshipType = "DESCRIBES"
      relatedSpdxElement = "SPDXRef-Package-AgentNightWatch"
    },
    [ordered]@{
      spdxElementId = "SPDXRef-Package-AgentNightWatch"
      relationshipType = "CONTAINS"
      relatedSpdxElement = "SPDXRef-File-AgentNightWatchExe"
    }
  )
}

$sbom | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $sbomPath -Encoding utf8
