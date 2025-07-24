# Get command line arguments or use defaults
$inputImage = if ($args[0]) { $args[0] } else { "Images\flowers.png" }
$size = if ($args[1]) { $args[1] } else { "--size=16" }

$fullBinPath = ".\"

# Get all executables in the specified build directory
$executables = Get-ChildItem -Path $fullBinPath -Filter "*$exeSuffix.exe"

if ($executables.Count -eq 0) {
    Write-Host "Error: No executables found in $fullBinPath" -ForegroundColor Red
    Write-Host "Please build the $buildConfig configuration first using: make $buildConfig" -ForegroundColor Yellow
    exit 1
}

$arguments = @($inputImage, "$size")

Write-Host "=== Performance Measurement ===" -ForegroundColor Green
Write-Host "Input image: $inputImage" -ForegroundColor Cyan
Write-Host "Size: $size" -ForegroundColor Cyan
Write-Host "Found $($executables.Count) executable(s)" -ForegroundColor Cyan
Write-Host ""

foreach ($exe in $executables) {
    Write-Host "Measuring performance of $($exe.Name)..."
    
    # Run the executable 3 times and measure each execution
    $average = 0
    for ($i = 1; $i -le 3; $i++) {
        $result = Measure-Command {
            & $exe.FullName $arguments
        }
        
        # Format the time in a readable way
        $seconds = $result.TotalSeconds
        $average = $average + $seconds / 3
        
        Write-Host "Run ${i} | Time: ${seconds} s"
    }
    Write-Host ""
    Write-Host "AVERAGE: ${seconds} s"
    Write-Host ""
}