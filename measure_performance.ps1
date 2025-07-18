# Get all executables in the Debug directory
$executables = Get-ChildItem -Path ".\build\Release" -Filter "*.exe"
# Get command line arguments or use defaults
$inputImage = if ($args[0]) { $args[0] } else { "Images\flowers.png" }
$size = if ($args[1]) { $args[1] } else { "--size=16" }
$arguments = @($inputImage, "$size")

foreach ($exe in $executables) {
    Write-Host "`nMeasuring performance of $($exe.Name)..."
    
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
} 