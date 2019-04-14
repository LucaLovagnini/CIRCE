/*
 * SIFTVLFeat.cpp
 *
 *  Created on: 24 Dec 2016
 *      Author: luca
 */

#include "SIFTVLFeat.hpp"

namespace cc{
	void siftForFun(IplImage &i_image, cv::Mat1f &descriptors, vl_sift_options &opts)
	{

		int imHeight, imWidth;
		imHeight = i_image.height;
		imWidth = i_image.width;

		//Take IplImage -> convert to SINGLE (float):
		float* frame = (float*)malloc(imHeight*imWidth*sizeof(float));
		uchar* Ldata = (uchar*)i_image.imageData;
		int ws = i_image.widthStep;
		int chns = i_image.nChannels;


		for(int i=0; i<imHeight; i++)
		{
			for(int j=0; j<imWidth; j++)
			{
				frame[j*imHeight+i*chns] = (float)Ldata[i*ws+j*chns];
			}
		}

		// vlsift computation:
		vl_sift_pix const *data;
		int M, N;
		data = (vl_sift_pix*)frame;
		M = i_image.height;
		N = i_image.width;

		// parse vlsift parameters
		int                _verbose				= opts.verbose; // change to 2 for more verbose..
		int                _O					= opts.O; //Octaves
		int                _S					= opts.S; //Levels
		int                _o_min				= opts.o_min;
		double             _edge_thresh			= opts.edge_thresh;  //-1 will use the default (as in matlab)
		double             _peak_thresh			= opts.peak_thresh;
		double             _norm_thresh			= opts.norm_thresh;
		double             _magnif				= opts.magnif;
		double             _window_size			= opts.window_size;
		//double			   *ikeys				= 0;
	//    int                nikeys				= opts.nikeys;
		vl_bool            force_orientations	= opts.force_orientations;
		vl_bool            floatDescriptors		= opts.floatDescriptors;
	//	bool				_ikeys_provided		= opts.ikeys_provided;


		/* -----------------------------------------------------------------
		*                                                            Do job
		* -------------------------------------------------------------- */

		/* create a filter to process the image */
		VlSiftFilt	*filt = vl_sift_new (M, N, _O, _S, _o_min) ;

		if (_peak_thresh >= 0)
			vl_sift_set_peak_thresh(filt, _peak_thresh);
		if (_edge_thresh >= 0)
			vl_sift_set_edge_thresh(filt, _edge_thresh);
		if (_norm_thresh >= 0)
			vl_sift_set_norm_thresh(filt, _norm_thresh);
		if (_magnif      >= 0)
			vl_sift_set_magnif(filt, _magnif);
		if (_window_size >= 0)
			vl_sift_set_window_size(filt, _window_size);

		if (_verbose)
		{
			printf("vl_sift: filter settings:\n") ;
			printf("vl_sift:   octaves      (O)      = %d\n", vl_sift_get_noctaves(filt));
			printf("vl_sift:   levels       (S)      = %d\n", vl_sift_get_nlevels(filt));
			printf("vl_sift:   first octave (o_min)  = %d\n", vl_sift_get_octave_first(filt));
			printf("vl_sift:   edge thresh           = %g\n", vl_sift_get_edge_thresh(filt));
			printf("vl_sift:   peak thresh           = %g\n", vl_sift_get_peak_thresh(filt));
			printf("vl_sift:   norm thresh           = %g\n", vl_sift_get_norm_thresh(filt));
			printf("vl_sift:   window size           = %g\n", vl_sift_get_window_size(filt));
			printf("vl_sift:   float descriptor      = %d\n", floatDescriptors);
	//		printf("vl_sift: input keys provided? %s\n", _ikeys_provided ? "yes" : "no") ;
	//		printf((nikeys >= 0) ? "vl_sift: will source frames? yes (%d read)\n" : "vl_sift: will source frames? no\n", nikeys);
			printf("vl_sift: will force orientations? %s\n", force_orientations ? "yes" : "no");
		}
		/* ...............................................................
		*                                             Process each octave
		* ............................................................ */

		if (vl_sift_process_first_octave(filt,data)!=VL_ERR_EOF){
			while (1){

				/* Run detector ............................................. */
				vl_sift_detect(filt);
				VlSiftKeypoint const *keys  = vl_sift_get_keypoints(filt);
				int nkeys = vl_sift_get_nkeypoints(filt);

				/* For each keypoint ........................................ */
				for (int i=0;i<nkeys;++i) {
					double angles [4];
					int	nangles=vl_sift_calc_keypoint_orientations(filt,angles,keys+i) ;

					for (int q=0 ; q < nangles ; ++q) {
						vl_sift_pix descr[128] ;
						vl_sift_calc_keypoint_descriptor(filt,descr, keys+i, angles [q]) ;
						cv::Mat1f d(1, 128, descr);
						d = 512*d;
						descriptors.push_back(d);
					}
				} /* next keypoint */
				if (vl_sift_process_next_octave(filt) == VL_ERR_EOF)
					break;
			} /* next octave */
		}

		/* cleanup */
		vl_sift_delete(filt);
		/* end: do job */
		return;
	}

	void SIFTVLFeat::mapParams(std::vector<std::pair<std::string,std::string>> &params){
		params.push_back(std::pair<std::string,std::string>("Descriptor","SIFTVLFeat"));
	}

	void SIFTVLFeat::ComputeDescriptors(const cv::Mat &img, cv::Mat1f &descriptors){
		IplImage iplimg = img;
		vl_sift_options options;
		siftForFun(iplimg, descriptors, options);
	}
}
