#include <FaceEngine.h>

#include <iostream>
#include <vector>

#include "io_util.h"

int main(int argc, char *argv[])
{
    // Parse command line arguments.
    // Arguments:
    // 1) path to a first descriptor,
    // 2) path to a second descriptor,
    // 3) matching threshold.
    // If matching score is above the threshold, then both descriptors
    // belong to the same person, otherwise they belong to different persons.
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

    std::clog << "firstDescriptorPath: \"" << firstDescriptorPath << "\"" << std::endl;
    std::clog << "secondDescriptorPath: \"" << secondDescriptorPath << "\"" << std::endl;
    std::clog << "threshold: " << threshold << std::endl;

    // Create config FaceEngine root SDK object.
    fsdk::ISettingsProviderPtr config;
    config = fsdk::acquire(fsdk::createSettingsProvider("./data/faceengine.conf"));
    if (!config) {
        std::cerr << "Failed to load face engine config instance." << std::endl;
        return -1;
    }
    config->setValue("DescriptorFactory::Settings", "model", 46);

    // Create FaceEngine root SDK object.
    fsdk::IFaceEnginePtr faceEngine = fsdk::acquire(fsdk::createFaceEngine("./data", "./data/faceengine.conf"));
    if (!faceEngine) {
        std::cerr << "Failed to create face engine instance." << std::endl;
        return -1;
    }
    
    if (faceEngine->getFaceEngineEdition() != fsdk::FaceEngineEdition::CompliteEdition) {
        std::cerr << "FaceEngine SDK Frontend edition doesn't support face descriptors. Use FaceEngine SDK Complite edition" << std::endl;
        return -1;
    }
    
    faceEngine->setSettingsProvider(config);

    // Create descriptor matcher.
    fsdk::IDescriptorMatcherPtr descriptorMatcher = fsdk::acquire(faceEngine->createMatcher());
    if (!descriptorMatcher) {
        std::cerr << "Failed to create face descriptor matcher instance." << std::endl;
        return -1;
    }

    // Create face descriptors.
    fsdk::IDescriptorPtr descriptor1 = fsdk::acquire(faceEngine->createDescriptor());
    fsdk::IDescriptorPtr descriptor2 = fsdk::acquire(faceEngine->createDescriptor());
    if (!descriptor1 || !descriptor2) {
        std::cerr << "Failed to create face descriptors instance." << std::endl;
        return -1;
    }

    // Load face descriptors.
    std::vector<uint8_t> data;

    data = std::move(readFile(firstDescriptorPath));
    VectorArchive vectorArchive1(data);
    if (!descriptor1->load(&vectorArchive1)) {
        std::cerr << "Failed to load face descriptor to vector." << std::endl;
        return -1;
    }

    data = std::move(readFile(secondDescriptorPath));
    VectorArchive vectorArchive2(data);
    if (!descriptor2->load(&vectorArchive2)) {
        std::cerr << "Failed to load face descriptor to vector." << std::endl;
        return -1;
    }

    // Descriptors similarity.
    float similarity;

    // Match 2 descriptors.
    // Returns similarity in range (0..1],
    // where: 0 means totally different.
    //        1 means totally the same.
    fsdk::ResultValue<fsdk::FSDKError, fsdk::MatchingResult> descriptorMatcherResult =
            descriptorMatcher->match(descriptor1, descriptor2);
    if (descriptorMatcherResult.isError()) {
        std::cerr << "Failed to match. Reason: " << descriptorMatcherResult.what() << std::endl;
        return -1;
    }

    similarity = descriptorMatcherResult.getValue().similarity;
    std::clog << "Derscriptors matched with score: " << similarity * 100.f << "%" << std::endl;

    // Check if similarity is above theshold.
    if (similarity > threshold)
        std::cout << "Descriptors belong to one person." << std::endl;
    else
        std::cout << "Descriptors belong to different persons." << std::endl;

    return 0;
}
