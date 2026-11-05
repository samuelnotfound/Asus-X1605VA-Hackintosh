param([string]$EfiPath = (Join-Path $PSScriptRoot '../EFI'))
$ErrorActionPreference = 'Stop'
$efi = (Resolve-Path -LiteralPath $EfiPath).Path
$oc = Join-Path $efi 'OC'
[xml]$config = Get-Content -LiteralPath (Join-Path $oc 'config.plist') -Raw
$problems = [Collections.Generic.List[string]]::new()
function Value($dict, [string]$key) {
    $dict.SelectSingleNode("key[.='$key']/following-sibling::*[1]")
}
function CheckFile([string]$relative) {
    if (-not (Test-Path -LiteralPath (Join-Path $oc $relative) -PathType Leaf)) {
        $problems.Add("Missing enabled file: $relative")
    }
}
$root = $config.plist.dict
foreach ($spec in @(@('ACPI', 'Add', 'Path', 'ACPI'), @('UEFI', 'Drivers', 'Path', 'Drivers'), @('Misc', 'Tools', 'Path', 'Tools'))) {
    $section = Value $root $spec[0]
    $entries = Value $section $spec[1]
    foreach ($entry in $entries.SelectNodes('dict')) {
        if ((Value $entry 'Enabled').Name -eq 'true') {
            CheckFile (Join-Path $spec[3] (Value $entry $spec[2]).InnerText)
        }
    }
}
$ids = @{}
$kernel = Value (Value $root 'Kernel') 'Add'
foreach ($entry in $kernel.SelectNodes('dict')) {
    $bundle = (Value $entry 'BundlePath').InnerText
    if ($bundle -match '(?i)(itlwm|itlmt7902|AirportItlwm|AirportBrcm|BrcmPatchRAM)') {
        $problems.Add("Wireless entry remains: $bundle")
    }
    if ((Value $entry 'Enabled').Name -ne 'true') { continue }
    $base = Join-Path 'Kexts' $bundle
    foreach ($key in 'PlistPath', 'ExecutablePath') {
        $relative = (Value $entry $key).InnerText
        if ($relative) { CheckFile (Join-Path $base $relative) }
    }
    $infoPath = Join-Path $oc (Join-Path $base (Value $entry 'PlistPath').InnerText)
    if (Test-Path -LiteralPath $infoPath) {
        [xml]$info = Get-Content -LiteralPath $infoPath -Raw
        $id = (Value $info.plist.dict 'CFBundleIdentifier').InnerText
        if ($id -and $ids.ContainsKey($id)) { $problems.Add("Duplicate enabled bundle ID: $id") }
        if ($id) { $ids[$id] = $bundle }
    }
}
$identity = Value (Value $root 'PlatformInfo') 'Generic'
foreach ($key in 'SystemSerialNumber', 'MLB', 'SystemUUID') {
    if ((Value $identity $key).InnerText) { $problems.Add("Private identity remains: $key") }
}
if ((Value $identity 'ROM').InnerText.Trim() -ne 'AAAAAAAA') { $problems.Add('ROM is not the zero placeholder') }
$boot = $config.SelectSingleNode("//key[.='boot-args']/following-sibling::string[1]").InnerText
if ($boot -match 'mt7902') { $problems.Add('MediaTek boot argument remains') }
foreach ($file in Get-ChildItem -LiteralPath $efi -Recurse -Force -File) {
    if ($file.Name -match '^(Background|Wallpaper).*\.(icns|png|jpe?g|bmp)$') { $problems.Add("Wallpaper remains: $($file.Name)") }
    if ($file.Name -like '._*') { $problems.Add("AppleDouble metadata remains: $($file.Name)") }
}
foreach ($bundle in Get-ChildItem -LiteralPath (Join-Path $oc 'Kexts') -Recurse -Directory -Filter '*.kext') {
    if ($bundle.Name -match '(?i)(itlwm|itlmt7902|AirportItlwm|AirportBrcm|BrcmPatchRAM)') { $problems.Add("Wireless bundle remains: $($bundle.Name)") }
}
CheckFile 'OpenCore.efi'
if (-not (Test-Path -LiteralPath (Join-Path $efi 'BOOT/BOOTX64.EFI'))) { $problems.Add('Missing BOOTX64.EFI') }
if ($problems.Count) { $problems | ForEach-Object { Write-Output "FAIL: $_" }; exit 1 }
Write-Output "PASS: enabled references, bundle IDs, sanitized identity, wireless and wallpaper checks ($($ids.Count) enabled kext bundles)."
Write-Output 'Hardware behavior and OpenCore schema/version compatibility require separate validation.'
