$SDL_VERSION = "2.0.8"
$SDL_TTF_VERSION = "2.0.14"
$SDL_MIXER_VERSION = "2.0.2"

#https://stackoverflow.com/questions/9948517/how-to-stop-a-powershell-script-on-the-first-error
$ErrorActionPreference = "Stop"

#https://stackoverflow.com/questions/27768303/how-to-unzip-a-file-in-powershell
Add-Type -AssemblyName System.IO.Compression.FileSystem

function Unzip {
    param([string]$zipfile, [string]$outpath)
    [System.IO.Compression.ZipFile]::ExtractToDirectory($zipfile, $outpath)
}

function Do-Sdl {
    param([string]$url, [string]$libName, [string]$version)
    echo " > Downloading $url"
    #Invoke-WebRequest -UseBasicParsing -Uri "$url" -OutFile "$libName.zip" #It is terribly slow
    #https://blog.jourdant.me/post/3-ways-to-download-files-with-powershell 2nd way
    (New-Object System.Net.WebClient).DownloadFile($url, "$libName.zip")
    echo " > Decompressing $libName.zip"
    Unzip "$libName.zip" .
    Rename-Item -Path $libName-$version -NewName "$libName"
    echo " > Deleting $libName.zip"
    Remove-Item "$LibName.zip"
}

Remove-Item -Recurse SDL*

Do-Sdl "https://www.libsdl.org/release/SDL2-devel-$SDL_VERSION-VC.zip" SDL2 $SDL_VERSION
Do-Sdl "https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-$SDL_TTF_VERSION-VC.zip" SDL2_ttf $SDL_TTF_VERSION
Do-SDL "https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-devel-$SDL_MIXER_VERSION-VC.zip" SDL2_mixer $SDL_MIXER_VERSION

echo " > Everything done"