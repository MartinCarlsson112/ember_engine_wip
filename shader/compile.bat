@echo off
for %%i in (*.vert *.frag *.tesc *.tese *.rmiss *.rgen *.rchit) do  %VULKAN_SDK%/Bin/glslc.exe --target-env=vulkan1.2 --target-spv=spv1.5  "%%~i" -o "%%~i.spv