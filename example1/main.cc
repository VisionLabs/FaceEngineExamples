#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <FaceEngine.h>
#include <Core/Log.h>

using namespace fsdk;

// Helper function to load images in PPM format.
// Read more about PPM here: https://en.wikipedia.org/wiki/Netpbm_format
// Some freeware tools that support conversion to/from PPM: Gimp, XnView.
Image loadImage(const char* path);

// Factory objects.
// These set up various SDK components.
// Note: SDK defines smart pointers for various object types named like IInterfacePtr.
// Such objects implement reference counting to manage their life time. The smart pointers
// will ensure that reference counting functions are called appropriately and the objects
// are properly destroyed after use.

IFaceEnginePtr g_engine;                       // Root SDK object.
IDetectorFactoryPtr g_detectorFactory;         // Face detector factory.
IFeatureFactoryPtr g_featureFactory;           // Facial feature detector factory.
IDescriptorFactoryPtr g_descriptorFactory;     // Facial descriptor factory.

// SDK components that we will use for facial recognition.
IDetectorPtr g_faceDetector;                   // Face detector.
IFeatureDetectorPtr g_featureDetector;         // Facial feature detector.
IDescriptorExtractorPtr g_descriptorExtractor; // Facial descriptor extractor.
IDescriptorMatcherPtr g_descriptorMatcher;     // Descriptor matcher.

// initialize FaceEngine.
bool initFaceEngine() {
   assert(!g_engine);
   assert(!g_detectorFactory);
   assert(!g_featureFactory);
   assert(!g_descriptorFactory);

   g_engine = acquire(createFaceEngine());

   if(!g_engine) {
      log::error("Failed to create face engine instance.");
      return false;
   }

   g_detectorFactory = acquire(g_engine->createDetectorFactory());

   if(!g_detectorFactory) {
      log::error("Failed to create face detector factory instance.");
      return false;
   }

   g_featureFactory = acquire(g_engine->createFeatureFactory());

   if(!g_featureFactory) {
      log::error("Failed to create face feature factory instance.");
      return false;
   }

   g_descriptorFactory = acquire(g_engine->createDescriptorFactory());

   if(!g_descriptorFactory) {
      log::error("Failed to create face descriptor factory instance.");
      return false;
   }

   g_faceDetector = acquire(g_detectorFactory->createDetector(ODT_DEFAULT));

   if(!g_faceDetector) {
      log::error("Failed to create face detector instance.");
      return false;
   }

   g_featureDetector = acquire(g_featureFactory->createDetector(FT_DEFAULT));

   if(!g_featureDetector) {
      log::error("Failed to create feature detector instance.");
      return false;
   }

   g_descriptorExtractor = acquire(g_descriptorFactory->createExtractor(DT_DEFAULT));

   if(!g_descriptorExtractor) {
      log::error("Failed to create descriptor extractor instance.");
      return false;
   }

   g_descriptorMatcher = acquire(g_descriptorFactory->createMatcher(DT_DEFAULT));

   if(!g_descriptorMatcher) {
      log::error("Failed to create descriptor matcher instance.");
      return false;
   }

   return true;
}

// Facial feature detection confidence threshold.
// We use this value to reject bad face detections.
const float g_confidenceThreshold = 0.25f;

// Extract a face descriptor for matching.
IDescriptorPtr
extractDescriptor(const Image& image) {

   assert(g_faceDetector);
   assert(g_featureDetector);
   assert(g_descriptorExtractor);

   if(!image) {
      log::error("Request image is invalid.");
      return nullptr;
   }

   // We will need both color and rayscale images.
   Image imageBGR;
   Image imageR;

   image.convert(imageBGR, Format::B8G8R8);
   image.convert(imageR, Format::R8);

   if(!imageBGR) {
      log::error("Conversion to BGR has failed.");
      return nullptr;
   }

   if(!imageR) {
      log::error("Conversion to grayscale has failed.");
      return nullptr;
   }

   // Stage 1. Detect a face.
   log::debug("Detecting faces.");

   // We assume no more than 10 faces per image.
   enum { MaxDetections = 10 };

   Detection detections[MaxDetections];
   int detectionCount(MaxDetections);

   // Detect up to 10 faces.
   // detectionCount will store actual number of faces found.
   if(auto r = g_faceDetector->detect(imageR, imageR.getRect(), &detections[0], &detectionCount)) {
      if(detectionCount) {

         log::debug("Found %d face(s).", detectionCount);

         // Stage 2. Detect facial features and compute a confidence score.
         IFeatureSetPtr bestFeatureSet(nullptr);
         int bestDetectionIndex(0);

         // Loop through all the faces and find one with the best score.
         for(auto detectionIndex = 0; detectionIndex < detectionCount; ++detectionIndex) {

            auto featureSet = acquire(g_featureFactory->createFeatureSet());

            log::debug("Detecting facial features (%d/%d).", (detectionIndex + 1), detectionCount);

            if(auto r = g_featureDetector->detect(imageR, detections[detectionIndex], featureSet)) {

               if(!bestFeatureSet || featureSet->getConfidence() > bestFeatureSet->getConfidence()) {
                  bestFeatureSet = featureSet;
                  bestDetectionIndex = detectionIndex;
               }
            }
            else {
               log::warn("Failed to detect feature set; skipping. Reason: %s", r.what());
               continue;
            }
         }

         // If we could not detect facial features OR feature confidence score is too low, abort.
         if(!bestFeatureSet || bestFeatureSet->getConfidence() < g_confidenceThreshold) {
            log::info("Face detection succeeded, but no faces with good confidence found.");
            return nullptr;
         }

         log::debug("Best face confidence is %0.3f.", bestFeatureSet->getConfidence());

         auto bestDetection = detections[bestDetectionIndex];

         // Stage 3. Create a face descriptor.
         auto descriptor = acquire(g_descriptorFactory->createDescriptor(DT_DEFAULT));

         log::debug("Extracting descriptor.");

         // This is typically the most time consuming task so we would like to do it only when sure
         // that our image is good enough for recognition purposes.
         if(auto r = g_descriptorExtractor->extract(imageBGR, bestDetection, bestFeatureSet, descriptor)) {

            log::info("Descriptor extracted successfully.");
            return descriptor;
         }
         else {
            log::error("Failed to extract face descriptor. Reason: %s", r.what());
            return nullptr;
         }
      }
      else {
         log::info("Face detection succeeded, but no faces found.");
         return nullptr;
      }
   }
   else {
      log::error("Face detection has failed. Reason: %s", r.what());
      return nullptr;
   }
}


// Match 2 descriptors.
// Returns similarity in range (0..1],
// where: 0 means totally different.
//        1 means totally the same.
float
matchDescriptors(IDescriptorPtr first, IDescriptorPtr second) {
   float similarity(0.f);

   if(auto r = g_descriptorMatcher->match(first, second)) {
      log::debug("Derscriptors matched with score: %1.1f%%",
         r.getValue().similarity * 100.f);

      similarity = r.getValue().similarity;
   }
   else {
      log::error("Failed to match. Reason: %s.", r.what());
   }

   return similarity;
}


int main(int argc, char *argv[]) {

   // Parse command line arguments.
   // We expect 3 of them:
   // 1) path to a first image to match
   // 2) path to a second image to match
   // 3) matching threshold
   // If matching score is above the threshold, we will conclude that both images
   // are of the same person. Otherwise, they're different.

   char* firstImagePath;
   char* secondImagePath;
   float threshold = 0.f;

   if(argc != 4) {
      log::info(
         "Usage: %s <image1.ppm> <image2.ppm> <threshold>\n\n"
         "image1.ppm - path to first image\n"
         "image2.ppm - path to second image\n"
         "threshold - similarity threshold in range (0..1]\n", argv[0]);
      return 1;
   }
   else {
      firstImagePath = argv[1];
      secondImagePath = argv[2];
      threshold = (float)atof(argv[3]);
   }

   log::debug("firstImagePath: \"%s\".", firstImagePath);
   log::debug("secondImagePath: \"%s\".", secondImagePath);
   log::debug("threshold: %1.3f.", threshold);

   // Initialize FaceEngine.
   if(!initFaceEngine()) {
      log::error("Failed to initialize FaceEngine.");
      return -1;
   }

   // Load images.
   auto image1 = loadImage(firstImagePath);
   auto image2 = loadImage(secondImagePath);

   if(!image1 || !image2) {
      log::error("Could not load images.");
      return -2;
   }

   // Extract face descriptors.
   auto desc1 = extractDescriptor(image1);
   auto desc2 = extractDescriptor(image2);

   if(!desc1 || !desc2) {
      log::error("Could not extract descriptors.");
      return -2;
   }

   // Match the descriptors and determine similarity.
   float similarity = matchDescriptors(desc1, desc2);

   // Check if similarity is above theshold.
   if(similarity > threshold)
      log::info("These persons look the same.");
   else
      log::info("These persons appear different.");

   return 0;
}

/*
 * This function was taken from demos archive at www.cs.rit.edu by Nate Robins and Nan Schaller.
 *
 * ppmRead: read a PPM raw (type P6) file.  The PPM file has a header
 * that should look something like:
 *
 *   P6
 *   # comment
 *   width height max_value
 *   rgbrgbrgb...
 *
 * where "P6" is the magic cookie which identifies the file type and
 * should be the only characters on the first line followed by a
 * carriage return.  Any line starting with a # mark will be treated
 * as a comment and discarded.  After the magic cookie, three integer
 * values are expected: width, height of the image and the maximum
 * value for a pixel (max_value must be < 256 for PPM raw files).  The
 * data section consists of width*height rgb triplets (one byte each)
 * in binary format (i.e., such as that written with fwrite() or
 * equivalent).
 *
 * The rgb data is returned as an array of unsigned chars (packed
 * rgb).  The malloc()'d memory should be free()'d by the caller.  If
 * an error occurs, an error message is sent to stderr and NULL is
 * returned.
 *
 */
unsigned char* ppmRead(const char* filename, int* width, int* height) {

   FILE* fp;

   int i, w, h, d;

   unsigned char* image;

   char head[70];    // max line <= 70 in PPM (per spec).

   fp = fopen( filename, "rb" );
   if ( !fp ) {
      perror(filename);
      return NULL;
   }

   // Grab first two chars of the file and make sure that it has the
   // correct magic cookie for a raw PPM file.
   fgets(head, 70, fp);
   if (strncmp(head, "P6", 2)) {
      fprintf(stderr, "%s: Not a raw PPM file\n", filename);
      return NULL;
   }

   // Grab the three elements in the header (width, height, maxval).
   i = 0;
   while( i < 3 ) {
      fgets( head, 70, fp );
      if ( head[0] == '#' )      // skip comments.
         continue;
      if ( i == 0 )
         i += sscanf( head, "%d %d %d", &w, &h, &d );
      else if ( i == 1 )
         i += sscanf( head, "%d %d", &h, &d );
      else if ( i == 2 )
         i += sscanf( head, "%d", &d );
   }

   // Grab all the image data in one fell swoop.
   image = (unsigned char*) malloc( sizeof( unsigned char ) * w * h * 3 );

   fread( image, sizeof( unsigned char ), w * h * 3, fp );
   fclose( fp );

   *width = w;
   *height = h;

   return image;
}

Image loadImage(const char* path) {
   Image image;

   // Load image from file.
   int w, h;
   auto bytes = ppmRead(path, &w, &h);

   // CHeck errors.
   if(!bytes) {
      log::error("Failed to load image from \"%s\".", path);
      return image;
   }

   // Initialize SDK Image object by copying raw bytes.
   // Image will manage it's memory automatically.
   image.create(w, h, Format::R8G8B8, bytes, true);

   // We no longer need this.
   free(bytes);

   return image;
}
