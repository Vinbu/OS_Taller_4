#include <iostream>
#include <pthread.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <unistd.h>

using namespace cv;
using namespace std;

/**
 * @brief Data structure for passing parameters to thread functions.
 */
struct ThreadData {
    Mat image;         
    String outputPath; 
    int threadIndex;   
    String opt;         // Processing option ("gray" or "format").
};

Mat convert_to_gray(const Mat& image) {
    Mat grayImage;
    cvtColor(image, grayImage, COLOR_BGR2GRAY);
    return grayImage;
}

/**
 * @brief Thread function that processes an image based on the provided option.
 * 
 * If opt is "gray", the image is converted to grayscale; if opt is "format",
 * the image is saved as is.
 * 
 * @param arg Pointer to ThreadData.
 * @return void* 
 */
void* thread_converter(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);

    cout << "Thread " << data->threadIndex << " started processing." << endl;

    if (data->opt == "gray") {
        Mat grayImage = convert_to_gray(data->image);
        imwrite(data->outputPath, grayImage);
    } else if (data->opt == "format") {
        imwrite(data->outputPath, data->image);
    }

    cout << "Thread " << data->threadIndex << " finished processing and saved " << data->outputPath << endl;

    delete data;
    pthread_exit(NULL);
}

/**
 * @brief Retrieves image file paths from the specified directory.
 * 
 * @param input Directory path or wildcard.
 * @return vector<cv::String> List of image file paths.
 */
vector<cv::String> get_images_path(const string& input) {
    vector<cv::String> paths;
    cout << "Searching in directory: " << input << endl;
    glob(input, paths, false);
    if (paths.empty()) {
        cout << "No images found in the specified directory." << endl;
    }
    return paths;
}

/**
 * @brief Loads images from file paths.
 * 
 * @param paths Vector of file paths.
 * @return vector<Mat> Vector of loaded images.
 */
vector<Mat> load_images(const vector<cv::String>& paths) {
    vector<Mat> images;
    for (size_t i = 0; i < paths.size(); i++) {
        Mat img = imread(paths[i]);
        if (img.empty()) {
            cout << "Could not open or find the image: " << paths[i] << endl;
            continue;
        }
        images.push_back(img);
    }
    return images;
}

/**
 * @brief Creates the output directory if it does not exist.
 * 
 * @param output Output directory path.
 * @return int 0 if successful, -1 otherwise.
 */
int create_output_directory(const string& output) {
    try {
        if (std::filesystem::create_directory(output)) {
            cout << "Directory created successfully: " << output << endl;
        } else {
            cout << "Directory already exists: " << output << endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        cerr << "Error creating directory: " << e.what() << endl;
        return -1;
    }
    return 0;
}

/**
 * @brief Processes images using multiple threads.
 * 
 * Loads images from the input directory, creates an output directory, and processes images
 * in batches using the specified number of threads. Depending on the option, the images are either
 * converted to grayscale ("gray") or saved in a different format ("format").
 *
 * The i + j index is used to identify the image of the current batch.
 * i: batch
 * j: image in the batch
 * 
 * @param paths Vector of paths of the images.
 * @param images Vector that contains the images.
 * @param output Output directory path.
 * @param num_threads Number of threads to use.
 * @param opt Processing option ("gray" or "format").
 * @param format (Optional) Extenxion to convert the image to when using -f.
 */
int thread_processing(vector<cv::String> paths, vector<Mat> images, String output, int num_threads, String opt, String format = "") { 
    size_t total = images.size();
    for (size_t i = 0; i < total; i += num_threads) {
        pthread_t threads[num_threads];
        int threadsCreated = 0;

        for (int j = 0; j < num_threads && (i + j) < total; j++) {
            std::filesystem::path imagePath(paths[i + j]);
            String image_name_format;
            if (opt == "gray") 
                image_name_format = imagePath.filename().string();
            if (opt == "format") 
                image_name_format = imagePath.stem().string();
            
            ThreadData* data = new ThreadData;
            data->image = images[i + j];
            data->outputPath = output + "/" + image_name_format + format;
            data->threadIndex = i + j;
            data->opt = opt;

            cout << "Main thread: Creating thread " << data->threadIndex << endl;
            int rc = pthread_create(&threads[j], NULL, thread_converter, (void*)data);
            if (rc) {
                cerr << "Error: unable to create thread, " << rc << endl;
                delete data;
                continue;
            }
            threadsCreated++;
        }

        for (int j = 0; j < threadsCreated; j++) {
            pthread_join(threads[j], NULL);
            cout << "Main thread: Joined thread " << (i + j) << endl;
        }
    }

    cout << "All images processed and saved." << endl;
    return 0;
}

/**
 * @brief Main function that processes command-line arguments and starts image processing.
 * 
 * Usage: ./program [-g | -f] -i input -o output -n threads [-t format]
 */
int main(int argc, char **argv) {
    int c;
    bool gFlag = false, fFlag = false;
    std::string input, output, format;
    int num_threads = 0;

    while ((c = getopt(argc, argv, "gfi:o:n:t:")) != -1) {
        switch (c) {
            case 'g':
                if (fFlag) {
                    std::cerr << "Error: -g cannot be used with -f" << std::endl;
                    return 1;
                }
                gFlag = true;
                break;
            case 'f':
                if (gFlag) {
                    std::cerr << "Error: -f cannot be used with -g" << std::endl;
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
            case 't':
                format = optarg;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-g | -f] -i input -o output -n threads" << std::endl;
                return 1;
        }
    }

    vector<cv::String> paths = get_images_path(input);
    vector<Mat> images = load_images(paths);
    create_output_directory(output);

    if (gFlag) 
        thread_processing(paths, images, output, num_threads, "gray");
    if (fFlag) {
        format = "." + format;
        thread_processing(paths, images, output, num_threads, "format", format);
    }

    std::cout << "Images processed correctly" << std::endl;
}

