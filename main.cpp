#include <iostream>
#include <pthread.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <unistd.h>

using namespace cv;
using namespace std;


// Each threadd will have its own data structure to hold the image and output path. 
struct ThreadData {
    Mat image;
    std::string outputPath;
    int threadIndex;
};

// Function to convert an image to grayscale.
Mat convert_to_gray(const Mat& image) {
    Mat grayImage;
    cvtColor(image, grayImage, COLOR_BGR2GRAY);
    return grayImage;
}

// Thread function that converts the image and saves the result.
void* thread_convert(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    
    // Print starting message.
    cout << "Thread " << data->threadIndex << " started processing." << endl;
    
    Mat gray = convert_to_gray(data->image);
    imwrite(data->outputPath, gray);
    
    // Print finishing message.
    cout << "Thread " << data->threadIndex << " finished processing and saved " << data->outputPath << endl;
    
    delete data;  // Clean up allocated memory.
    pthread_exit(nullptr);
}

int main(int argc, char **argv) {
    int c;
    bool gFlag = false, fFlag = false;
    std::string input, output;
    int num_threads = 0;

    while ((c = getopt(argc, argv, "gfi:o:n:")) != -1) {
        switch (c) {
            case 'g':
                if (fFlag) {
                    std::cerr << "Error: No se puede usar -g junto con -f" << std::endl;
                    return 1;
                }
                gFlag = true;
                break;

            case 'f':
                if (gFlag) {
                    std::cerr << "Error: No se puede usar -f junto con -g" << std::endl;
                    return 1;
                }
                fFlag = true;
                break;

            case 'i':
                input = optarg;
                break;

            case 'o':
                output = optarg;
                break;

            case 'n':
                num_threads = std::stoi(optarg);
                break;

            default:
                std::cerr << "Uso: " << argv[0] << " [-g | -f] -i input -o output -n threads" << std::endl;
                return 1;
        }
    }

    if (gFlag) {
        // Load image filenames.
        vector<cv::String> fn;
        cout << "Searching in directory: " << input << endl;
        glob(input, fn, false);
        if (fn.empty()) {
            cout << "No images found in the specified directory." << endl;
            return -1;
        }

        // Load images from the filenames.
        vector<Mat> images;
        for (size_t i = 0; i < fn.size(); i++) {
            Mat img = imread(fn[i]);
            if (img.empty()) {
                cout << "Could not open or find the image: " << fn[i] << endl;
                continue;
            }
            images.push_back(img);
        }

        // Create output directory.
        std::string outDir = output;
        try {
            if (std::filesystem::create_directory(outDir)) {
                cout << "Directory created successfully: " << outDir << endl;
            } else {
                cout << "Directory already exists: " << outDir << endl;
            }
        } catch (const std::filesystem::filesystem_error& e) {
            cerr << "Error creating directory: " << e.what() << endl;
            return -1;
        }

        // Process images in batches of threads concurrently.
        size_t total = images.size();
        for (size_t i = 0; i < total; i += num_threads) {
            pthread_t threads[num_threads];
            int threadsCreated = 0;
            
            // Create up to NUM_THREADS threads if there are images available.
            for (int j = 0; j < num_threads && (i + j) < total; j++) {
                // Prepare thread data.
                ThreadData* data = new ThreadData;
                data->image = images[i + j];  // Copy the image.
                data->outputPath = outDir + "/gray_" + std::to_string(i + j) + ".jpg";
                data->threadIndex = i + j; // Assign a unique thread index.
                
                cout << "Main thread: Creating thread " << data->threadIndex << endl;
                // Create the thread.
                int rc = pthread_create(&threads[j], nullptr, thread_convert, (void*)data);
                if (rc) {
                    cerr << "Error: unable to create thread, " << rc << endl;
                    delete data;  // Clean up if thread creation fails.
                    continue;
                }
                threadsCreated++;
            }
            
            // Wait for the created threads to finish.
            for (int j = 0; j < threadsCreated; j++) {
                pthread_join(threads[j], nullptr);
                cout << "Main thread: Joined thread " << (i + j) << endl;
            }
        }

        cout << "All images processed and saved." << endl;
        return 0;
    }

    if (fFlag) {
        // Future Implementation
        return 0;
    }
    return 0;
}
