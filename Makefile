# Makefile for WaveFunctionCollapse - Windows compatible version
# Usage: make all, make clean, make help

# Configuration definitions
CONFIGS := x86-Debug x86-Release x64-Debug x64-Release

# Default target
.PHONY: all
all: $(CONFIGS) package

# Help target
.PHONY: help
help:
	@echo Available targets:
	@echo   all          - Build all configurations (Debug/Release for x86/x64)
	@echo   x86-Debug    - Build 32-bit Debug version
	@echo   x86-Release  - Build 32-bit Release version
	@echo   x64-Debug    - Build 64-bit Debug version
	@echo   x64-Release  - Build 64-bit Release version
	@echo   package      - Create distributable packages for all configs
	@echo   package-<config> - Create package for specific config
	@echo   clean        - Remove all build directories
	@echo   clean-config - Remove specific build directory
	@echo   help         - Show this help message
	@echo.
	@echo Examples:
	@echo   make all
	@echo   make x64-Release
	@echo   make package
	@echo   make package-x64-Release
	@echo   make clean-x86-Debug

# Build all configurations
$(CONFIGS): %: build/%/bin/$(if $(findstring Debug,$*),Debug,Release)/main_seq_Q$(if $(findstring Debug,$*),_d,).exe

# Rule to build each configuration
build/%/bin/$(if $(findstring Debug,$*),Debug,Release)/main_seq_Q$(if $(findstring Debug,$*),_d,).exe:
	@echo Building $*...
	@powershell -Command "if (!(Test-Path 'build')) { New-Item -ItemType Directory -Path 'build' | Out-Null }"
	@powershell -Command "if (!(Test-Path 'build/$*')) { New-Item -ItemType Directory -Path 'build/$*' | Out-Null }"
	@cmake -B build/$* -A $(if $(findstring x86,$*),Win32,x64) -DCMAKE_BUILD_TYPE=$(if $(findstring Debug,$*),Debug,Release)
	@cmake --build build/$* --config $(if $(findstring Debug,$*),Debug,Release)
	@echo $* build completed successfully

# Package targets
.PHONY: package
package: package-x86-Debug package-x86-Release package-x64-Debug package-x64-Release

# Individual package targets
package-x86-Debug: build/x86-Debug/bin/Debug/main_seq_Q_d.exe
	@echo Creating package for x86-Debug...
	@powershell -Command "$$binDir = 'build/x86-Debug/bin/Debug'; $$packageDir = 'packages/x86-Debug'; if (!(Test-Path $$packageDir)) { New-Item -ItemType Directory -Path $$packageDir | Out-Null }; Copy-Item -Path $$binDir/*.exe -Destination $$packageDir -Force; (Get-Content 'measure_performance.ps1') -replace 'x64-Release', 'x86-Debug' | Set-Content (Join-Path $$packageDir 'measure_performance.ps1'); Copy-Item -Path 'Images' -Destination $$packageDir -Recurse -Force;"

package-x86-Release: build/x86-Release/bin/Release/main_seq_Q.exe
	@echo Creating package for x86-Release...
	@powershell -Command "$$binDir = 'build/x86-Release/bin/Release'; $$packageDir = 'packages/x86-Release'; if (!(Test-Path $$packageDir)) { New-Item -ItemType Directory -Path $$packageDir | Out-Null }; Copy-Item -Path $$binDir/*.exe -Destination $$packageDir -Force; (Get-Content 'measure_performance.ps1') -replace 'x64-Release', 'x86-Release' | Set-Content (Join-Path $$packageDir 'measure_performance.ps1'); Copy-Item -Path 'Images' -Destination $$packageDir -Recurse -Force; Compress-Archive -Path $$packageDir -DestinationPath 'WaveFunctionCollapse-x86-Release.zip' -Force; Write-Host 'Package created: WaveFunctionCollapse-x86-Release.zip'"

package-x64-Debug: build/x64-Debug/bin/Debug/main_seq_Q_d.exe
	@echo Creating package for x64-Debug...
	@powershell -Command "$$binDir = 'build/x64-Debug/bin/Debug'; $$packageDir = 'packages/x64-Debug'; if (!(Test-Path $$packageDir)) { New-Item -ItemType Directory -Path $$packageDir | Out-Null }; Copy-Item -Path $$binDir/*.exe -Destination $$packageDir -Force; (Get-Content 'measure_performance.ps1') -replace 'x64-Release', 'x64-Debug' | Set-Content (Join-Path $$packageDir 'measure_performance.ps1'); Copy-Item -Path 'Images' -Destination $$packageDir -Recurse -Force;"

package-x64-Release: build/x64-Release/bin/Release/main_seq_Q.exe
	@echo Creating package for x64-Release...
	@powershell -Command "$$binDir = 'build/x64-Release/bin/Release'; $$packageDir = 'packages/x64-Release'; if (!(Test-Path $$packageDir)) { New-Item -ItemType Directory -Path $$packageDir | Out-Null }; Copy-Item -Path $$binDir/*.exe -Destination $$packageDir -Force; (Get-Content 'measure_performance.ps1') -replace 'x64-Release', 'x64-Release' | Set-Content (Join-Path $$packageDir 'measure_performance.ps1'); Copy-Item -Path 'Images' -Destination $$packageDir -Recurse -Force; Compress-Archive -Path $$packageDir -DestinationPath 'WaveFunctionCollapse-x64-Release.zip' -Force; Write-Host 'Package created: WaveFunctionCollapse-x64-Release.zip'"

# Clean targets
.PHONY: clean
clean:
	@echo Cleaning all build directories...
	@powershell -Command "if (Test-Path 'build') { Remove-Item 'build' -Recurse -Force }"
	@powershell -Command "if (Test-Path 'packages') { Remove-Item 'packages' -Recurse -Force }"
	@powershell -Command "Get-ChildItem -Filter 'WaveFunctionCollapse-*.zip' | Remove-Item -Force"
	@echo Clean completed

# Individual clean targets
clean-x86-Debug:
	@echo Cleaning x86-Debug...
	@powershell -Command "if (Test-Path 'build/x86-Debug') { Remove-Item 'build/x86-Debug' -Recurse -Force }"
	@powershell -Command "if (Test-Path 'packages/x86-Debug') { Remove-Item 'packages/x86-Debug' -Recurse -Force }"
	@powershell -Command "if (Test-Path 'WaveFunctionCollapse-x86-Debug.zip') { Remove-Item 'WaveFunctionCollapse-x86-Debug.zip' -Force }"

clean-x86-Release:
	@echo Cleaning x86-Release...
	@powershell -Command "if (Test-Path 'build/x86-Release') { Remove-Item 'build/x86-Release' -Recurse -Force }"
	@powershell -Command "if (Test-Path 'packages/x86-Release') { Remove-Item 'packages/x86-Release' -Recurse -Force }"
	@powershell -Command "if (Test-Path 'WaveFunctionCollapse-x86-Release.zip') { Remove-Item 'WaveFunctionCollapse-x86-Release.zip' -Force }"

clean-x64-Debug:
	@echo Cleaning x64-Debug...
	@powershell -Command "if (Test-Path 'build/x64-Debug') { Remove-Item 'build/x64-Debug' -Recurse -Force }"
	@powershell -Command "if (Test-Path 'packages/x64-Debug') { Remove-Item 'packages/x64-Debug' -Recurse -Force }"
	@powershell -Command "if (Test-Path 'WaveFunctionCollapse-x64-Debug.zip') { Remove-Item 'WaveFunctionCollapse-x64-Debug.zip' -Force }"

clean-x64-Release:
	@echo Cleaning x64-Release...
	@powershell -Command "if (Test-Path 'build/x64-Release') { Remove-Item 'build/x64-Release' -Recurse -Force }"
	@powershell -Command "if (Test-Path 'packages/x64-Release') { Remove-Item 'packages/x64-Release' -Recurse -Force }"
	@powershell -Command "if (Test-Path 'WaveFunctionCollapse-x64-Release.zip') { Remove-Item 'WaveFunctionCollapse-x64-Release.zip' -Force }"

# Status target to show what's built
.PHONY: status
status:
	@echo Build status:
	@powershell -Command "foreach ($$config in @('$(CONFIGS)')) { $$buildDir = \"build/$$config\"; $$binDir = if ($$config -like '*Debug*') { 'bin\\Debug' } else { 'bin\\Release' }; $$exeName = if ($$config -like '*Debug*') { 'main_seq_Q_d.exe' } else { 'main_seq_Q.exe' }; $$exePath = Join-Path $$buildDir $$binDir $$exeName; if (Test-Path $$exePath) { Write-Host \"  ✓ $$config - $$exePath\" } else { Write-Host \"  ✗ $$config - Not built\" } }"
	@echo.
	@echo Package status:
	@powershell -Command "foreach ($$config in @('$(CONFIGS)')) { $$zipFile = \"WaveFunctionCollapse-$$config.zip\"; if (Test-Path $$zipFile) { Write-Host \"  ✓ $$zipFile\" } else { Write-Host \"  ✗ $$zipFile - Not created\" } }"

# Show build info
.PHONY: info
info:
	@echo WaveFunctionCollapse Build System
	@echo ==================================
	@echo Configurations: $(CONFIGS)
	@echo Build directory: build/
	@echo Package directory: packages/
	@echo.
	@echo CMake version:
	@cmake --version
	@echo.
	@echo Available targets: make help