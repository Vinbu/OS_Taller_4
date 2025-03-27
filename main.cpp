#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream> 
#include <filesystem>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    // Read the image file
    Mat image = imread("../images/capi.jpg", IMREAD_COLOR);
    Mat grayImage;

    // Check for failure
    if (image.empty()) {
        cout << "Could not open or find the image" << endl;
        cin.get(); // wait for any key press
        return -1;
    }

    cvtColor(image, grayImage, COLOR_BGR2GRAY);

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


    imwrite("../out/graycapi.jpg", grayImage);
    cout << "Image saved (hopefully)" << endl;

    return 0;
}
