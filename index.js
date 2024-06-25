const fs = require('fs');
const path = require('path');

const unsupportedDpapi = {
    protectData() {
        throw new Error("Dpapi is not supported on this platform");
    },
    unprotectData() {
        throw new Error("Dpapi is not supported on this platform");
    }
}

function getDpapi() {
    if (process.platform !== "win32") {
        return unsupportedDpapi;
    }

    const fileName = 'node-dpapi.node';

    const localBuildPath = path.join(__dirname, 'build', 'Release', fileName)
    if (fs.existsSync(localBuildPath)) {
        return require(localBuildPath);
    }

    const arch = process.arch;
    switch (arch) {
        case 'arm64':
        case 'x64':
        case 'ia32':
            return require(path.join(__dirname, 'bin', arch, fileName));
        default:
            throw unsupportedDpapi;
    }
}

const dpapi = getDpapi();

module.exports.protectData = dpapi.protectData;
module.exports.unprotectData = dpapi.unprotectData;
