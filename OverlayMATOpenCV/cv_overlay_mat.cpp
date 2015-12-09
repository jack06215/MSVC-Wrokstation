#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>

void overlayImage(const cv::Mat &background, const cv::Mat &foreground, cv::Mat &output, cv::Point2i location);


int main(int argc, char* argv[])
{
	// add the second parameter "-1" as flag, to make sure the transparancy channel is read!
	cv::Mat foreground = cv::imread("bomb256.jpg");
	cv::Mat background = cv::imread("fig1.jpg");
	cv::Mat result;
	cv::Mat output;

	// Input Quadilateral or Image plane coordinates
	cv::Point2f inputQuad[4];
	// Output Quadilateral or World plane coordinates
	cv::Point2f outputQuad[4];

	cv::Mat lambda = cv::Mat::zeros(foreground.rows, foreground.cols, foreground.type());

	// The 4 points that select quadilateral on the input , from top-left in clockwise order
	// These four pts are the sides of the rect box used as input 
	inputQuad[0] = cv::Point2f(-30, -60);
	inputQuad[1] = cv::Point2f(foreground.cols + 50, -50);
	inputQuad[2] = cv::Point2f(foreground.cols + 100, foreground.rows + 50);
	inputQuad[3] = cv::Point2f(-50, foreground.rows + 50);
	// The 4 points where the mapping is to be done , from top-left in clockwise order
	outputQuad[0] = cv::Point2f(0, 0);
	outputQuad[1] = cv::Point2f(foreground.cols - 1, 0);
	outputQuad[2] = cv::Point2f(foreground.cols - 1, foreground.rows - 1);
	outputQuad[3] = cv::Point2f(0, foreground.rows - 1);

	// Get the Perspective Transform Matrix i.e. lambda 
	lambda = getPerspectiveTransform(inputQuad, outputQuad);
	// Apply the Perspective Transform just found to the src image
	warpPerspective(foreground, output, lambda, output.size());

	overlayImage(background, output, result, cv::Point(50, 0));
	cv::imshow("result", result);
	cv::waitKey();


	return 0;
}

void overlayImage(const cv::Mat &background, const cv::Mat &foreground, cv::Mat &output, cv::Point2i location)
{
	background.copyTo(output);
	// start at the row indicated by location, or at row 0 if location.y is negative.
	for (int y = std::max(location.y, 0); y < background.rows; ++y)
	{
		int fY = y - location.y; // because of the translation

		// we are done of we have processed all rows of the foreground image.
		if (fY >= foreground.rows) break;

		// start at the column indicated by location, 
		// or at column 0 if location.x is negative.
		for (int x = std::max(location.x, 0); x < background.cols; ++x)
		{
			int fX = x - location.x; // because of the translation.

			// we are done with this row if the column is outside of the foreground image.
			if (fX >= foreground.cols)
				break;

			// determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
			double opacity = ((double)foreground.data[fY * foreground.step + fX * foreground.channels() + 3]) / 255.;
			// and now combine the background and foreground pixel, using the opacity, 
			// but only if opacity > 0.
			for (int c = 0; opacity > 0 && c < output.channels(); ++c)
			{
				unsigned char foregroundPx = foreground.data[fY * foreground.step + fX * foreground.channels() + c];
				unsigned char backgroundPx = background.data[y * background.step + x * background.channels() + c];
				output.data[y * output.step + output.channels() * x + c] = backgroundPx * (1. - opacity) + foregroundPx * opacity;
			}
		}
	}
}