#include <FaceEngine.h>
#include <vlf/Log.h>

#include <iostream>
#include <vector>

#include "io_util.h"

int main(int argc, char *argv[])
{
    // Parse command line arguments.
    // We expect 3 of them:
    // 1) path to a first descriptor to match
    // 2) path to a second descriptor to match
    // 3) matching threshold
    // If matching score is above the threshold, we will conclude that both images
    // are of the same person. Otherwise, they're different.
    if(argc != 4) {
        std::cout << "Usage: "<<  argv[0] << " <descriptor1> <descriptor2> <threshold>\n"
                " *descriptor1 - path to first descriptor\n"
                " *descriptor2 - path to second descriptor\n"
                " *threshold - similarity threshold in range (0..1]\n"
                << std::endl;
        return -1;
    }
    char *firstDescriptorPath = argv[1];
    char *secondDescriptorPath = argv[2];
    float threshold = (float)atof(argv[3]);

    vlf::log::info("firstDescriptorPath: \"%s\".", firstDescriptorPath);
    vlf::log::info("secondDescriptorPath: \"%s\".", secondDescriptorPath);
    vlf::log::info("threshold: %1.3f.", threshold);

    // Create config FaceEngine root SDK object.
    fsdk::ISettingsProviderPtr config;
    config = fsdk::acquire(fsdk::createSettingsProvider("./data/faceengine.conf"));
    if (!config) {
        vlf::log::error("Failed to load face engine config.");
        return -1;
    }

    // Create FaceEngine root SDK object.
    fsdk::IFaceEnginePtr faceEngine = fsdk::acquire(fsdk::createFaceEngine(fsdk::CFF_OMIT_SETTINGS));
    if (!faceEngine) {
        vlf::log::error("Failed to create face engine instance.");
        return -1;
    }
    faceEngine->setSettingsProvider(config);
    faceEngine->setDataDirectory("./data/");

    // Create descriptor factory.
    fsdk::IDescriptorFactoryPtr descriptorFactory = fsdk::acquire(faceEngine->createDescriptorFactory());
    if (!descriptorFactory) {
        vlf::log::error("Failed to create face descriptor factory instance.");
        return -1;
    }

    // Create CNN descriptor matcher.
    fsdk::IDescriptorMatcherPtr descriptorMatcher =
            fsdk::acquire(descriptorFactory->createMatcher(fsdk::DT_CNN));
    if (!descriptorMatcher) {
        vlf::log::error("Failed to create face descriptor matcher instance.");
        return -1;
    }

    // Create CNN face descriptors.
    fsdk::IDescriptorPtr descriptor1 = fsdk::acquire(descriptorFactory->createDescriptor(fsdk::DT_CNN));
    fsdk::IDescriptorPtr descriptor2 = fsdk::acquire(descriptorFactory->createDescriptor(fsdk::DT_CNN));
    if (!descriptor1 || !descriptor2) {
        vlf::log::error("Failed to create face descriptors instance.");
        return -1;
    }

    // Load face descriptors.
    std::vector<uint8_t> data;

    data = std::move(readFile(firstDescriptorPath));
    VectorArchive vectorArchive1(data);
    if (!descriptor1->load(&vectorArchive1)) {
        vlf::log::error("Failed to load face descriptor to vector.");
        return -1;
    }

    data = std::move(readFile(secondDescriptorPath));
    VectorArchive vectorArchive2(data);
    if (!descriptor2->load(&vectorArchive2)) {
        vlf::log::error("Failed to load face descriptor to vector.");
        return -1;
    }

    // Match the descriptors and determine similarity.
    float similarity;

    // Match 2 descriptors.
    // Returns similarity in range (0..1],
    // where: 0 means totally different.
    //        1 means totally the same.
    fsdk::ResultValue<fsdk::FSDKError, fsdk::MatchingResult> descriptorMatcherResult =
            descriptorMatcher->match(descriptor1, descriptor2);
    if (descriptorMatcherResult.isError()) {
        vlf::log::error("Failed to match. Reason: %s.", descriptorMatcherResult.what());
        return -1;
    }

    similarity = descriptorMatcherResult.getValue().similarity;
    vlf::log::info("Derscriptors matched with score: %1.1f%%", similarity * 100.f);

    // Check if similarity is above theshold.
    if (similarity > threshold)
        std::cout << "These persons look the same." << std::endl;
    else
        std::cout << "These persons appear different." << std::endl;

    return 0;
}
