export type DataProtectionScope = "CurrentUser" | "LocalMachine";

declare module "node-dpapi-prebuilt" {
    function protectData(
        userData: Uint8Array,
        optionalEntropy: Uint8Array | null,
        scope: DataProtectionScope
    ): Uint8Array;

    function unprotectData(
        encryptedData: Uint8Array,
        optionalEntropy: Uint8Array | null,
        scope: DataProtectionScope
    ): Uint8Array;
}
