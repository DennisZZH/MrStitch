
// CPP program to Stitch 
// input images (panorama) using OpenCV  
#include <iostream> 
#include <fstream> 
  
// Include header files from OpenCV directory 
// required to stitch images. 
#include "opencv2/imgcodecs.hpp" 
#include "opencv2/highgui.hpp" 
#include "opencv2/stitching.hpp" 
#include <sys/time.h>	// for gettimeofday() 

using namespace cv;
  
// Define mode for stitching as panoroma  
// (One out of many functions of Stitcher) 
Stitcher::Mode mode = Stitcher::PANORAMA; 
  
// Array for pictures 
std::vector<Mat> imgs; 

int stitch_imgs(int num, char* jobname){

    struct timeval start, end;
	gettimeofday(&start, NULL);

    // Get all the images that need to be  
    // stitched as arguments from command line
    std::string imgname;
    std::string ext = ".jpg";
    for (int i = 0; i < num; ++i) 
    {       
            imgname = std::string(jobname) + std::to_string(i) + ext;
            // Read the ith argument or image  
            // and push into the image array 
            Mat img = imread(imgname); 
            if (img.empty()) 
            { 
                // Exit if image is not present 
                std::cout << "Can't read image '" << imgname << "'\n"; 
                return -1; 
            } 
            imgs.push_back(img); 
    } 
      
    // Define object to store the stitched image 
    Mat pano; 
      
    // Create a Stitcher class object with mode panoroma 
    Ptr<Stitcher> stitcher = Stitcher::create(mode); 
      
    // Command to stitch all the images present in the image array 
    Stitcher::Status status = stitcher->stitch(imgs, pano); 
  
    if (status != Stitcher::OK) 
    { 
        // Check if images could not be stiched 
        // status is OK if images are stiched successfully 
        std::cout << "Can't stitch images\n"; 
        return -1; 
    } 
      
    // Store a new image stiched from the given  
    //set of images as "result.jpg" 
    imwrite(std::string(jobname) + "_result" + ext, pano); 
      
    // Show the result 
    // imshow("Result", pano);  
    // waitKey(0); 

     // finish timing
    gettimeofday(&end, NULL);

	long seconds = (end.tv_sec - start.tv_sec);
	long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);

	printf("****************Stitching time is %ld micro second*****************\n", micros);

    return 0;
}