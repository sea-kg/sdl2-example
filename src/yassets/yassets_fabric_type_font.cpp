#include <yassets_fabric_type_font.h>

// ---------------------------------------------------------------------
// YAssetFactoryFont

YAssetFactoryFont::YAssetFactoryFont(
    YAssetsService *pAssetsService,
    YAssetFabricTypeFont *pFactoryTypeFont,
    const std::string &sImagePath,
    const std::string &sAlphabet
) : YAssetFactory(pAssetsService, pFactoryTypeFont) {
    TAG = "YAssetFactoryFont";
    m_pFactoryTypeFont = pFactoryTypeFont;
    m_sImagePath = sImagePath;
    m_sAlphabet = sAlphabet;
    m_pTexture = pAssetsService->getRenderWindow()->loadTexture(m_sImagePath);
}

YAsset *YAssetFactoryFont::createAsset() {
    YLog::throw_err(TAG, "TODO");
    return nullptr;
}

// ---------------------------------------------------------------------
// YAssetFabricTypeFont

YAssetFabricTypeFont::YAssetFabricTypeFont(YAssetsService *pAssetsService) 
    : YAssetFactoryType(pAssetsService) {
    TAG = "YAssetFabricTypeFont";
}

std::string YAssetFabricTypeFont::getFabricTypeId() {
    return "font";
}

YAssetFactory *YAssetFabricTypeFont::createFactory(
    const std::string &sAssetFactoryPath,
    const std::string &sFactoryId,
    const YJsonObject &jsonFactoryConfig
) {
    std::string sImagePath = sAssetFactoryPath + "/" + jsonFactoryConfig["font-image"].getString();
    if (!YCore::fileExists(sImagePath)) {
        YLog::throw_err(TAG, "Did not font-image " + sImagePath);
    }
    std::string sAlphabet = jsonFactoryConfig["font-alphabet"].getString();

    return new YAssetFactoryFont(
        m_pAssetsService, 
        this,
        sImagePath,
        sAlphabet
    );
}
