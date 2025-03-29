#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream> 
#include <filesystem>
#include <opencv2/core/utility.hpp>

using namespace cv;
using namespace std;

Mat convert_to_gray(const Mat& image) {
    Mat grayImage;
    cvtColor(image, grayImage, COLOR_BGR2GRAY);
    return grayImage;
}

int main(int argc, char **argv) {
    /*
     * Gets the paths of the images that match the pattern.
     * and stores them in the vector fn.
     */
    vector<cv::String> fn;
    glob("../images/*", fn, false);
    if (fn.empty()) {
        cout << "No images found in the specified directory." << endl;
        return -1;
    }

    /*
     * Reads the images from the paths stored in the vector fn.
     * and stores them in the vector images.
     */
    vector<Mat> images;
    size_t count = fn.size();
    for (size_t i = 0; i < count; i++) {
        Mat img = imread(fn[i]);
        if (img.empty()) {
            cout << "Could not open or find the image: " << fn[i] << endl;
            continue;
        }
        images.push_back(img);
    }

    // prints the images` paths for debugging purposes
    for (size_t i = 0; i < count; i++) {
        cout << "Image " << i << ": " << fn[i] << endl;
    }

    // Creates a directory to save the images
    std::string path = "../out"; // Specify the folder name 
    try { 
        if (std::filesystem::create_directory(path)) { 
            std::cout << "Directory created successfully: " << path << std::endl; 
        } else { 
            std::cout << "Directory already exists: " << path << std::endl; 
        } 
    } catch (const std::filesystem::filesystem_error& e) { 
        std::cerr << "Error creating directory: " << e.what() << std::endl; 
    }

    // Save the gray image
    // TODO: save the image with the same name as the original image
    for (size_t i = 0; i < count; i++) {
        Mat grayImg = convert_to_gray(images[i]);
        std::string outputPath = path + "/gray_" + std::to_string(i) + ".jpg";
        imwrite(outputPath, grayImg);
    }

    cout << "Images saved (hopefully)" << endl;

    return 0;
}
