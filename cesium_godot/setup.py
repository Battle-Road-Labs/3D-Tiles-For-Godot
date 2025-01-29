import subprocess
import os
import sys
import fnmatch


CESIUM_NATIVE_DIR = "native"
OS_WIN = "nt"

initialPath: str = ''
buildConfig: str = "Release"


# Clone the native repo
def clone_native():
    if os.path.exists(CESIUM_NATIVE_DIR):
        print("Detectec Cesium is already cloned, skipping cloning...")
        return
    print('Cloning Cesium Native')
    # Clone the url
    URL = "https://github.com/CesiumGS/cesium-native.git"
    subprocess.run(["git", "clone", URL, "--recursive", CESIUM_NATIVE_DIR])

def clone_libcurl():
    pass

# Configure with CMake
def configure():
    os.chdir(CESIUM_NATIVE_DIR)
    # Assume you already have the triplet (for now)
    triplet = "x64-windows-static"
    os.environ["VCPKG_TRIPLET"] = triplet
    # Run Cmake with the /MT flag on
    result = subprocess.run(
        ["cmake", "-DCESIUM_MSVC_STATIC_RUNTIME_ENABLED=ON", "-DVCPKG_TRIPLET=%s" % triplet, "."])

    # We pray this works haha
    if result.returncode != 0:
        errorMsg = "cmake return code: %s" % str(result.returncode)
        print('Error configuring Cesium native, please make sure you have CMake installed and up to date: ' + errorMsg)
        return
    print("Configuration completed without any errors!")


def compile():
    print("Compiling Cesium Native...")
    if os.name == OS_WIN:
        # execute MSBuild
        solutionName: str = "cesium-native.sln"
        msbuildPath: str = find_ms_build()
        if msbuildPath == '':
            print(
                "Could not find MSBuild.exe, make sure to have Visual Studio installed", file=sys.stderr)
            return
        releaseConfig = "/property:Configuration=%s" % buildConfig
        result = subprocess.run([msbuildPath, solutionName, releaseConfig])
        if result.returncode != 0:
            print("Error building Cesium Native: %s" % str(result.stderr))
        return
    print("Compiling for platform %s is not yet supported!" %
          os.name, file=sys.stderr)


def configure_libraries():
    # Find all the directories that start with "Cesium"
    cesiumDirs: list[str] = [x for x in os.listdir() if os.path.isdir(x) and 'Cesium' in x]
    # Now, inside each directory we can find a .lib
    # We need to rename the lib to match the platform and build type
    for dir in cesiumDirs:
        moduleName = os.path.basename(os.path.normpath(dir))
        toSearch = "%s*.lib" % (moduleName)
        found, libPath = find_in_dir_recursive(dir, toSearch)
        if not found:
            print("Could not find module built library with name %s, try to recompile native" % moduleName)
            continue
        newFileName = "%s.windows.editor.dev.x86_64.lib" % moduleName
        rawPath = os.path.dirname(os.path.normpath(libPath))
        newPath = os.path.join(rawPath, newFileName)
        os.rename(libPath, newPath)


def find_ms_build() -> str:
    print("Searching for MS Build")
    # Try to search for an msbuild executable in the system
    try:
        testCmd = subprocess.run(
            ["msbuild", "-version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        # Yay, we found it... (not gonna happen lol)
        if testCmd.returncode == 0:
            return 'msbuild'
    except:
        # More likely we'll need to search for another path
        vsPath = "C:\\Program Files\\Microsoft Visual Studio"
        found, path = find_in_dir_recursive(vsPath, '*MSBuild.exe')

        if found:
            # Access the next latest directory (latest VS version)
            return path

        # Try with a .NET path
        print(".NET path is not yet supported!", sys.stderr)
        return ''


def try_find_next_dir(path: str) -> (bool, str):
    foundEntries: list[str] = os.listdir(path)
    res: list[str] = [x for x in foundEntries if os.path.isdir(x)]
    if len(res) == 0:
        return False, ''
    return True, res[0]


def find_in_dir_recursive(path: str, pattern: str) -> (bool, str):
    """
    Use only when there might be a few directories left to search
    as this function is recursive
    """

    if not os.path.exists(path):
        return False, ''

    foundFiles: list[str] = os.listdir(path)

    if len(foundFiles) == 0:
        return False, ''

    for root, dirnames, filenames in os.walk(path):
        for filename in fnmatch.filter(filenames, pattern):
            return True, os.path.join(root, filename)

    return False, ''


def apply_setup():
    if len(sys.argv) > 1:
        global buildConfig
        buildConfig = sys.argv[1]

    global initialPath
    initialPath = os.getcwd()

    clone_native()
    configure()
    compile()
    configure_libraries()


apply_setup()
