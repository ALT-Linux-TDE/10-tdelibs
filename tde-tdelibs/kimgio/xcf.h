#ifndef XCF_H
#define XCF_H
/*
 * qxcfi.cpp: A Qt 3 plug-in for reading GIMP XCF image files
 * Copyright (C) 2001 lignum Computing, Inc. <allen@lignumcomputing.com>
 * Copyright (C) 2004 Melchior FRANZ <mfranz@kde.org>
 *
 * This plug-in is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <tqimage.h>
#include <tqiodevice.h>
#include <tqvaluestack.h>
#include <tqvaluevector.h>

#include "gimp.h"


extern "C" {
void kimgio_xcf_read(TQImageIO *);
void kimgio_xcf_write(TQImageIO *);
}

const float INCHESPERMETER = (100.0 / 2.54);

/*!
 * Each layer in an XCF file is stored as a matrix of
 * 64-pixel by 64-pixel images. The GIMP has a sophisticated
 * method of handling very large images as well as implementing
 * parallel processing on a tile-by-tile basis. Here, though,
 * we just read them in en-masse and store them in a matrix.
 */
typedef TQValueVector<TQValueVector<TQImage> > Tiles;



class XCFImageFormat {
public:
	XCFImageFormat();
	void readXCF(TQImageIO* image_io);


private:
	/*!
	 * Each GIMP image is composed of one or more layers. A layer can
	 * be one of any three basic types: RGB, grayscale or indexed. With an
	 * optional alpha channel, there are six possible types altogether.
	 *
	 * Note: there is only ever one instance of this structure. The
	 * layer info is discarded after it is merged into the final TQImage.
	 */
	class Layer {
	public:
		TQ_UINT32 width;			//!< Width of the layer
		TQ_UINT32 height;		//!< Height of the layer
		TQ_INT32 type;			//!< Type of the layer (GimpImageType)
		char* name;			//!< Name of the layer
		TQ_UINT32 hierarchy_offset;	//!< File position of Tile hierarchy
		TQ_UINT32 mask_offset;		//!< File position of mask image

		uint nrows;			//!< Number of rows of tiles (y direction)
		uint ncols;			//!< Number of columns of tiles (x direction)

		Tiles image_tiles;		//!< The basic image
		//! For Grayscale and Indexed images, the alpha channel is stored
		//! separately (in this data structure, anyway).
		Tiles alpha_tiles;
		Tiles mask_tiles;		//!< The layer mask (optional)

		//! Additional information about a layer mask.
		struct {
			TQ_UINT32 opacity;
			TQ_UINT32 visible;
			TQ_UINT32 show_masked;
			uchar red, green, blue;
			TQ_UINT32 tattoo;
		} mask_channel;

		bool active;			//!< Is this layer the active layer?
		TQ_UINT32 opacity;		//!< The opacity of the layer
		TQ_UINT32 visible;		//!< Is the layer visible?
		TQ_UINT32 linked;		//!< Is this layer linked (geometrically)
		TQ_UINT32 preserve_transparency; //!< Preserve alpha when drawing on layer?
		TQ_UINT32 apply_mask;		//!< Apply the layer mask?
		TQ_UINT32 edit_mask;		//!< Is the layer mask the being edited?
		TQ_UINT32 show_mask;		//!< Show the layer mask rather than the image?
		TQ_INT32 x_offset;		//!< x offset of the layer relative to the image
		TQ_INT32 y_offset;		//!< y offset of the layer relative to the image
		TQ_UINT32 mode;			//!< Combining mode of layer (LayerModeEffects)
		TQ_UINT32 tattoo;		//!< (unique identifier?)

		//! As each tile is read from the file, it is buffered here.
		uchar tile[TILE_WIDTH * TILE_HEIGHT * sizeof(TQRgb)];

		//! The data from tile buffer is copied to the Tile by this
		//! method.  Depending on the type of the tile (RGB, Grayscale,
		//! Indexed) and use (image or mask), the bytes in the buffer are
		//! copied in different ways.
		void (*assignBytes)(Layer& layer, uint i, uint j);

		Layer(void) : name(0) {}
		~Layer(void) { delete[] name; }
	};


	/*!
	 * The in-memory representation of the XCF Image. It contains a few
	 * metadata items, but is mostly a container for the layer information.
	 */
	class XCFImage {
	public:
		TQ_UINT32 width;			//!< width of the XCF image
		TQ_UINT32 height;		//!< height of the XCF image
		TQ_INT32 type;			//!< type of the XCF image (GimpImageBaseType)

		TQ_UINT8 compression;		//!< tile compression method (CompressionType)
		float x_resolution;		//!< x resolution in dots per inch
		float y_resolution;		//!< y resolution in dots per inch
		TQ_INT32 tattoo;			//!< (unique identifier?)
		TQ_UINT32 unit;			//!< Units of The GIMP (inch, mm, pica, etc...)
		TQ_INT32 num_colors;		//!< number of colors in an indexed image
		TQValueVector<TQRgb> palette;	//!< indexed image color palette

		int num_layers;			//!< number of layers
		Layer layer;			//!< most recently read layer

		bool initialized;		//!< Is the TQImage initialized?
		TQImage image;			//!< final TQImage

		XCFImage(void) : initialized(false) {}
	};


	//! In layer DISSOLVE mode, a random number is chosen to compare to a
	//! pixel's alpha. If the alpha is greater than the random number, the
	//! pixel is drawn. This table merely contains the random number seeds
	//! for each ROW of an image. Therefore, the random numbers chosen
	//! are consistent from run to run.
	static int random_table[RANDOM_TABLE_SIZE];

	//! This table provides the add_pixel saturation values (i.e. 250 + 250 = 255).
	//static int add_lut[256][256]; - this is so lame waste of 256k of memory
	static int add_lut( int, int );

	//! The bottom-most layer is copied into the final TQImage by this
	//! routine.
	typedef void (*PixelCopyOperation)(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);

	//! Higher layers are merged into the the final TQImage by this routine.
	typedef void (*PixelMergeOperation)(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);

	//! Layer mode static data.
	typedef struct {
		bool affect_alpha;		//!< Does this mode affect the source alpha?
	} LayerModes;

	//! Array of layer mode structures for the modes described by
	//! LayerModeEffects.
	static const LayerModes layer_modes[];

	bool loadImageProperties(TQDataStream& xcf_io, XCFImage& image);
	bool loadProperty(TQDataStream& xcf_io, PropType& type, TQByteArray& bytes);
	bool loadLayer(TQDataStream& xcf_io, XCFImage& xcf_image);
	bool loadLayerProperties(TQDataStream& xcf_io, Layer& layer);
	bool composeTiles(XCFImage& xcf_image);
	void setGrayPalette(TQImage& image);
	void setPalette(XCFImage& xcf_image, TQImage& image);
	static void assignImageBytes(Layer& layer, uint i, uint j);
	bool loadHierarchy(TQDataStream& xcf_io, Layer& layer);
	bool loadLevel(TQDataStream& xcf_io, Layer& layer, TQ_INT32 bpp);
	static void assignMaskBytes(Layer& layer, uint i, uint j);
	bool loadMask(TQDataStream& xcf_io, Layer& layer);
	bool loadChannelProperties(TQDataStream& xcf_io, Layer& layer);
	bool initializeImage(XCFImage& xcf_image);
	bool loadTileRLE(TQDataStream& xcf_io, uchar* tile, int size,
			int data_length, TQ_INT32 bpp);
	static void copyLayerToImage(XCFImage& xcf_image);
	static void copyRGBToRGB(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);

	static void copyGrayToGray(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);
	static void copyGrayToRGB(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);
	static void copyGrayAToRGB(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);
	static void copyIndexedToIndexed(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);
	static void copyIndexedAToIndexed(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);
	static void copyIndexedAToRGB(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);

	static void mergeLayerIntoImage(XCFImage& xcf_image);
	static void mergeRGBToRGB(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);
	static void mergeGrayToGray(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);
	static void mergeGrayAToGray(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);
	static void mergeGrayToRGB(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);
	static void mergeGrayAToRGB(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);
	static void mergeIndexedToIndexed(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);
	static void mergeIndexedAToIndexed(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);
	static void mergeIndexedAToRGB(Layer& layer, uint i, uint j, int k, int l,
			TQImage& image, int m, int n);

	static void dissolveRGBPixels(TQImage& image, int x, int y);
	static void dissolveAlphaPixels(TQImage& image, int x, int y);
};

#endif
