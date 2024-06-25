declare module "node-dpapi-prebuilt" {
    function protectData(
        userData: Uint8Array,
        optionalEntropy: Uint8Array | null,
        scope: "CurrentUser" | "LocalMachine"
    ): Uint8Array;

    function unprotectData(
        encryptedData: Uint8Array,
        optionalEntropy: Uint8Array | null,
        scope: "CurrentUser" | "LocalMachine"
    ): Uint8Array;
}
