# Get command line arguments or use defaults
$buildConfig = if ($args[0]) { $args[0] } else { "x64-Release" }
$inputImage = if ($args[1]) { $args[1] } else { "Images\flowers.png" }
$size = if ($args[2]) { $args[2] } else { "--size=16" }

# Validate build configuration
$validConfigs = @("x86-Debug", "x86-Release", "x64-Debug", "x64-Release")
if ($buildConfig -notin $validConfigs) {
    Write-Host "Error: Invalid build configuration '$buildConfig'" -ForegroundColor Red
    Write-Host "Valid configurations: $($validConfigs -join ', ')" -ForegroundColor Yellow
    Write-Host "Usage: .\measure_performance.ps1 [buildConfig] [inputImage] [size]" -ForegroundColor Yellow
    Write-Host "Example: .\measure_performance.ps1 x64-Release Images\flowers.png --size=16" -ForegroundColor Yellow
    exit 1
}

# Determine the correct build directory and executable suffix
$buildDir = "build\$buildConfig"
$binDir = if ($buildConfig -like "*Debug*") { "bin\Debug" } else { "bin\Release" }
$exeSuffix = if ($buildConfig -like "*Debug*") { "_d" } else { "" }
$fullBinPath = Join-Path $buildDir $binDir

# Check if build directory exists
if (!(Test-Path $fullBinPath)) {
    Write-Host "Error: Build directory not found: $fullBinPath" -ForegroundColor Red
    Write-Host "Please build the $buildConfig configuration first using: make $buildConfig" -ForegroundColor Yellow
    exit 1
}

# Get all executables in the specified build directory
$executables = Get-ChildItem -Path $fullBinPath -Filter "*$exeSuffix.exe"

if ($executables.Count -eq 0) {
    Write-Host "Error: No executables found in $fullBinPath" -ForegroundColor Red
    Write-Host "Please build the $buildConfig configuration first using: make $buildConfig" -ForegroundColor Yellow
    exit 1
}

$arguments = @($inputImage, "$size")

Write-Host "=== Performance Measurement for $buildConfig ===" -ForegroundColor Green
Write-Host "Input image: $inputImage" -ForegroundColor Cyan
Write-Host "Size: $size" -ForegroundColor Cyan
Write-Host "Build directory: $fullBinPath" -ForegroundColor Cyan
Write-Host "Found $($executables.Count) executable(s)" -ForegroundColor Cyan
Write-Host ""

foreach ($exe in $executables) {
    Write-Host "Measuring performance of $($exe.Name)..."
    
    # Run the executable 3 times and measure each execution
    for ($i = 1; $i -le 3; $i++) {
        $result = Measure-Command {
            & $exe.FullName $arguments
        }
        
        # Format the time in a readable way
        $minutes = [math]::Floor($result.TotalMinutes)
        $seconds = $result.TotalSeconds % 60
        $milliseconds = $result.TotalMilliseconds % 1000
        
        Write-Host "Run ${i} | Time: ${seconds}s"
    }
    Write-Host ""
}