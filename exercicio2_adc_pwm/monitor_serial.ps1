$env:IDF_PATH = 'C:\Users\User\esp\v5.2.1\esp-idf'
$env:IDF_TOOLS_PATH = 'C:\Espressif'
$env:IDF_PYTHON_ENV_PATH = 'C:\Espressif\tools\python\v5.2.1\venv'
$env:IDF_PYTHON_CHECK_CONSTRAINTS = 'no'

$idfTools = @(
    'C:\Espressif\tools\cmake\3.24.0\bin'
    'C:\Espressif\tools\ninja\1.11.1'
    'C:\Espressif\tools\xtensa-esp-elf\esp-13.2.0_20230928\xtensa-esp-elf\bin'
    'C:\Espressif\tools\ccache\4.8'
) -join ';'

$env:PATH = "$idfTools;$env:PATH"

& 'C:\Espressif\tools\python\v5.2.1\venv\Scripts\python.exe' `
  'C:\Users\User\esp\v5.2.1\esp-idf\tools\idf.py' `
  -B build_nortos -p COM6 monitor
