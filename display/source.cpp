#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>

#include <opencv2/opencv.hpp>   
#include <opencv2/core/core.hpp>   
#include <opencv2/highgui/highgui.hpp>  

using namespace cv;

#define PI 3.14159265359

typedef struct {
	int r, g, b;
}int_rgb;

int** IntAlloc2(int height, int width)
{
	int** tmp;
	tmp = (int**)calloc(height, sizeof(int*));
	for (int i = 0; i<height; i++)
		tmp[i] = (int*)calloc(width, sizeof(int));
	return(tmp);
}

void IntFree2(int** image, int height, int width)
{
	for (int i = 0; i<height; i++)
		free(image[i]);

	free(image);
}

int_rgb** IntColorAlloc2(int height, int width)
{
	int_rgb** tmp;
	tmp = (int_rgb**)calloc(height, sizeof(int_rgb*));
	for (int i = 0; i<height; i++)
		tmp[i] = (int_rgb*)calloc(width, sizeof(int_rgb));
	return(tmp);
}

void IntColorFree2(int_rgb** image, int height, int width)
{
	for (int i = 0; i<height; i++)
		free(image[i]);

	free(image);
}

int** ReadImage(char* name, int* height, int* width)
{
	Mat img = imread(name, IMREAD_GRAYSCALE);
	int** image = (int**)IntAlloc2(img.rows, img.cols);

	*width = img.cols;
	*height = img.rows;

	for (int i = 0; i<img.rows; i++)
		for (int j = 0; j<img.cols; j++)
			image[i][j] = img.at<unsigned char>(i, j);

	return(image);
}

void WriteImage(char* name, int** image, int height, int width)
{
	Mat img(height, width, CV_8UC1);
	for (int i = 0; i<height; i++)
		for (int j = 0; j<width; j++)
			img.at<unsigned char>(i, j) = (unsigned char)image[i][j];

	imwrite(name, img);
}

void ImageShow(char* winname, int** image, int height, int width)
{
	Mat img(height, width, CV_8UC1);
	for (int i = 0; i<height; i++)
		for (int j = 0; j<width; j++)
			img.at<unsigned char>(i, j) = (unsigned char)image[i][j];
	imshow(winname, img);
	waitKey(0);
}

int_rgb** ReadColorImage(char* name, int* height, int* width)
{
	Mat img = imread(name, IMREAD_COLOR);
	int_rgb** image = (int_rgb**)IntColorAlloc2(img.rows, img.cols);

	*width = img.cols;
	*height = img.rows;

	for (int i = 0; i<img.rows; i++)
		for (int j = 0; j < img.cols; j++) {
			image[i][j].b = img.at<Vec3b>(i, j)[0];
			image[i][j].g = img.at<Vec3b>(i, j)[1];
			image[i][j].r = img.at<Vec3b>(i, j)[2];
		}

	return(image);
}

void WriteColorImage(char* name, int_rgb** image, int height, int width)
{
	Mat img(height, width, CV_8UC3);
	for (int i = 0; i<height; i++)
		for (int j = 0; j < width; j++) {
			img.at<Vec3b>(i, j)[0] = (unsigned char)image[i][j].b;
			img.at<Vec3b>(i, j)[1] = (unsigned char)image[i][j].g;
			img.at<Vec3b>(i, j)[2] = (unsigned char)image[i][j].r;
		}

	imwrite(name, img);
}

void ColorImageShow(char* winname, int_rgb** image, int height, int width)
{
	Mat img(height, width, CV_8UC3);
	for (int i = 0; i<height; i++)
		for (int j = 0; j<width; j++) {
			img.at<Vec3b>(i, j)[0] = (unsigned char)image[i][j].b;
			img.at<Vec3b>(i, j)[1] = (unsigned char)image[i][j].g;
			img.at<Vec3b>(i, j)[2] = (unsigned char)image[i][j].r;
		}
	imshow(winname, img);

}

template <typename _TP>
void ConnectedComponentLabeling(_TP** seg, int height, int width, int** label, int* no_label)
{

	//Mat bw = threshval < 128 ? (img < threshval) : (img > threshval);
	Mat bw(height, width, CV_8U);

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++)
			bw.at<unsigned char>(i, j) = (unsigned char)seg[i][j];
	}
	Mat labelImage(bw.size(), CV_32S);
	*no_label = connectedComponents(bw, labelImage, 8); // 0까지 포함된 갯수임

	(*no_label)--;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++)
			label[i][j] = labelImage.at<int>(i, j);
	}
}

// First Image Processing
void Ex0305() {
	/** Image Pointer */
	int** image;

	/** width, height of image */
	int height, width;
	
	// Initialize
	image = ReadImage("koala.jpg", &height, &width);

	// Calculate
	for (int x = 0, y = 0; x < width; x++)
	{
		// the equation of y for x
		y = (int)(0.4 * x + 100 + 0.5);

		// check index out of range
		if (y < 0 || y > height - 1) continue;
		else image[y][x] = 255;
	}
	
	// Show result
	ImageShow("test", image, height, width);
}

/** 2*2 Matrix
 * @argument a (Row 1, Column 1)
 * @argument b (Row 1, Column 2)
 * @argument c (Row 2, Column 1)
 * @argument d (Row 2, Column 2)
*/
typedef struct {
	double a, b;
	double c, d;
}Matrix2X2;

Matrix2X2 GetInverseMatrix(double a, double b, double c, double d)
{
	double determinant = a*d - b*c;
	Matrix2X2 MatrixOut = { d / determinant, -b / determinant, -c / determinant, a / determinant };

	return MatrixOut;
}

Matrix2X2 GetInverseMatrix(Matrix2X2 matrix)
{
	return GetInverseMatrix(matrix.a, matrix.b, matrix.c, matrix.d);
}

/** Affine transform function
* @param Input (x, y) coordinate
* @param a,b,c,d,t1,t2,x1,y2 affine transform arguments
*/
Point2d Affine(Point2d Input, double a, double b, double c, double d, double t1 = 0, double t2 = 0, double x1 = 0, double y1 = 0)
{
	double NewX = a*(Input.x - x1) + b*(Input.y - y1) + t1;
	double NewY = c*(Input.x - x1) + d*(Input.y - y1) + t2;

	return Point2d(NewX, NewY);
}

/** Affine transform function
* @param Input (x, y) coordinate
* @param matrix includes a, b, c, d arguments
* @param a,b,c,d,t1,t2,x1,y2 affine transform arguments
*/
Point2d Affine(Point2d Input, Matrix2X2 matrix, double t1 = 0, double t2 = 0, double x1 = 0, double y1 = 0)
{
	double NewX = matrix.a * (Input.x - x1) + matrix.b * (Input.y - y1) + t1;
	double NewY = matrix.c * (Input.x - x1) + matrix.d * (Input.y - y1) + t2;

	return Point2d(NewX, NewY);
}

/** Affine transform function
* @param Input (x, y, w) coordinate
* @param a, b, t1 NewX = a * OldX + b * OldY + t1
* @param c, d, t2 NewY = c * OldX + d * OldY + t2
*/
Point3d Affine(Point3d Input, double a, double b, double c, double d, double t1 = 0, double t2 = 0, double x1 = 0, double y1 = 0)
{
	double NewX = a*(Input.x - x1) + b*(Input.y - y1) + t1;
	double NewY = c*(Input.x - x1) + d*(Input.y - y1) + t2;
	double NewW = 1;

	return Point3d(NewX, NewY, NewW);
}

/** Draw a line
 * @param Image Image file that stores color of each pixel
 * @param Height height of image
 * @parma Width width of image
 * @param a Inclination of line
 * @param b Constant added to formula
 * @param Thickness Thickness of line
 * @param brightness Brightness of line
 *
 * @formula --> y = ax + b --> ax - y + b = 0
 *          --> d = |ax0 - y0 + b| / sqrt(a*a + 1)
 */
int** DrawLine(int** Image, int Height, int Width, double a, double b, double Thickness, uint8_t brightness) {
	int** ImageOut = IntAlloc2(Height, Width);

	for (int y = 0; y < Height; y++)
		for (int x = 0; x < Width; x++)
		{
			double d = fabs(a*x - y + b) / sqrt(a*a + 1.0);

			if (d < Thickness) Image[y][x] = brightness;
		}

	return ImageOut;
}

/** Draw a circle filled with 
 * @param Image Image file that stores color of each pixel
 * @param Height height of image
 * @parma Width width of image
 * @param a X coordinate of center of circle
 * @param b Y coordinate of center of circle
 * @param r radius of circle
 * @param brightness Brightness of circle

 * @formula --> (x - a)^2 + (y - b)^2 = r^2
 */
int** DrawCircle(int** Image, int Height, int Width, double a, double b, double r, uint8_t brightness) {
	int** ImageOut = IntAlloc2(Height, Width);
	
	for (int y = 0; y < Height; y++)
		for (int x = 0; x < Width; x++)
		{
			if (powl(x - a, 2) + powl(y - b, 2) <= powl(r, 2))
				ImageOut[y][x] = brightness;
			else
				ImageOut[y][x] = Image[y][x];
		}

	return ImageOut;
}

/** Apply bilinear interpolation on image after affine transform
* @param Image image to apply bilinear interpolation on
* @param Height height of image
* @param Width width of image
* @param a, b, c, d arguments of invertible matrix
*/
int** BilinearInterpolation(int** Image, int Height, int Width, double a, double b, double c, double d, double t1 = 0, double t2 = 0, double x1 = 0, double y1 = 0)
{
	int** ImageOut = IntAlloc2(Height, Width);

	for (int y = 0; y < Height; y++)
		for (int x = 0; x < Width; x++)
		{
			Point2d InversedPoint = Affine(Point2d(x, y), GetInverseMatrix(a, b, c, d), t1, t2, x1, y1);

			int LT_X = (int)InversedPoint.x;	int RT_X = LT_X + 1;
			int LT_Y = (int)InversedPoint.y;	int RT_Y = LT_Y;
			int LB_X = LT_X;					int RB_X = LT_X + 1;
			int LB_Y = LT_Y + 1;				int RB_Y = LT_Y + 1;

			// pass sequence if image processcing area isn't inside of image size
			if (LT_X < 0 || LT_Y < 0 || RB_X > Width - 1 || RB_Y > Height - 1)
				continue;

			double dx = InversedPoint.x - LT_X;
			double dy = InversedPoint.y - LT_Y;

			int A = Image[LT_Y][LT_X];			int B = Image[RT_Y][RT_X];
			int C = Image[LB_Y][LB_X];			int D = Image[RB_Y][RB_X];

			double value =
				A * (1.0 - dx) * (1.0 - dy) +
				B * dx * (1.0 - dy) +
				C * (1.0 - dx) * dy +
				D * dx * dy;

			ImageOut[y][x] = (int)(value + 0.5);
		}

	return ImageOut;
}

 /** Rotate image clockwise and apply bilinear interpolation on image
  * @param Image image to rotate
  * @param Height height of image
  * @param Width width of image
  * @param Angle rotation angle
  * @param OriginY y coordinate of origin for rotation
  * @param OriginX x coordinate of origin for rotation
  */
 int** RotationTransform(int** Image, double Height, double Width, double Angle, double OriginY = 0, double OriginX = 0)
 {
	 // transform radian to degree
	 Angle /= 57.2958;

	 return BilinearInterpolation(Image, Height, Width, cos(Angle), -sin(Angle), sin(Angle), cos(Angle), OriginX, OriginY, OriginX, OriginY);;
 }

int main()
{
	/** Image Pointer */
	int** OriginalImage;
	int** DrawingImage;
	int** AffinedImage;
	int** BilinearInterpolationImage;
	int** RotatedImage;

	/** width, height of image */
	int Height, Width;

	/** Initialize */
	OriginalImage = ReadImage("koala.jpg", &Height, &Width);
	DrawingImage = IntAlloc2(Height, Width);
	AffinedImage = IntAlloc2(Height, Width);
	BilinearInterpolationImage = IntAlloc2(Height, Width);
	RotatedImage = IntAlloc2(Height, Width);

	/** Image Processing */

	// Drawing Circle
	DrawingImage = DrawCircle(OriginalImage, Height, Width, 200, 200, 80, 220);
	DrawingImage = DrawCircle(OriginalImage, Height, Width, 350, 350, 40, 120);
	
	// Affine Transform
	for (int y = 0; y < Height - 1; y++)
		for (int x = 0; x < Width - 1; x++)
		{
			Point2d point(x, y);
			Point2d point_out = Affine(point, 1.0, 0.4, 0.4, 2.0);

			if (point_out.y > Height - 1 || point_out.y < 0 || point_out.x > Width - 1 || point_out.x < 0) continue;
			AffinedImage[(int)(point_out.y+0.5)][(int)(point_out.x+0.5)] = OriginalImage[y][x];
		}

	// Bilinear Interpolaion
	BilinearInterpolationImage = BilinearInterpolation(OriginalImage, Height, Width, 1.5, 0, 0, 1.5);
	
	// Rotation Transform
	RotatedImage = RotationTransform(OriginalImage, Height, Width, 45, Height/2, Width/2);

	/** Show Image */
	ImageShow("Original Image", OriginalImage, Height, Width);
	ImageShow("Drawing Image", DrawingImage, Height, Width);
	ImageShow("Affined Image", AffinedImage, Height, Width);
	ImageShow("Bilinear Interpolation Image", BilinearInterpolationImage, Height, Width);
	ImageShow("Rotated Image", RotatedImage, Height, Width);

	return 0;
}