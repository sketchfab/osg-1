// grabbed a small implemetation of LZMA by Lloyd Hilaiel, based on the original work by Igor Pavlov
// I considered LZMA due to it providing better compression ratio than gzip, at the cost of slower compression/decompression.

// after adding lzma, i also saw Google's Draco, seems to use edgebreaker, thought I could combine draco and lzma, but would need more time.

#include <osg/Notify>
#include <osgDB/Registry>
#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <sstream>

#include <easylzma/compress.h>
#include <easylzma/decompress.h>

struct dataStream
{
	const unsigned char * inData;
	size_t inLen;

	unsigned char * outData;
	size_t outLen;
};

static int
inputCallback(void *ctx, void *buf, size_t * size)
{
	size_t rd = 0;
	struct dataStream * ds = (struct dataStream *) ctx;

	rd = (ds->inLen < *size) ? ds->inLen : *size;

	if (rd > 0) 
	{
		// copy data chunks into LZMA's temp buffer
		memcpy(buf, (void *)ds->inData, rd);
		ds->inData += rd;
		ds->inLen -= rd;
	}

	*size = rd;

	return 0;
}

static size_t
outputCallback(void *ctx, const void *buf, size_t size)
{
	struct dataStream * ds = (struct dataStream *) ctx;

	if (size > 0) 
	{
		// LZMA has finished with this chunk, copy it to the output
		ds->outData = (unsigned char*)realloc(ds->outData, ds->outLen + size);
		memcpy((void *)(ds->outData + ds->outLen), buf, size);
		ds->outLen += size;
	}

	return size;
}

void formatBytes(float inBytes, float& outBytes, std::string& outExt)
{
	static std::map<int, const char*> ExtMap = { { 0, "B" },{ 1, "KB" },{ 2, "MB" },{ 3, "GB" },{ 4, "TB" } };
	int extCounter = 0;
	outBytes = inBytes;
	while (outBytes > 1024.f)
	{
		outBytes = outBytes / 1024.f;
		extCounter++;
	} 
	outExt = ExtMap[extCounter];
}

class LZMACompressor : public osgDB::BaseCompressor
{
public:
	LZMACompressor() {}

	virtual bool compress(std::ostream& fout, const std::string& src)
	{
		clock_t start = clock();
		int rc;
		elzma_compress_handle hand = elzma_compress_alloc();
		
		// we see good enough gains with default settings, we could edit these for example giving the LP a higher value can give gains
		rc = elzma_compress_config(hand, ELZMA_LC_DEFAULT, ELZMA_LP_DEFAULT, ELZMA_PB_DEFAULT, 5, (1 << 20), ELZMA_lzip, src.size());

		if (rc != ELZMA_E_OK)
		{
			elzma_compress_free(&hand);
			return false;
		}

		struct dataStream ds;
		ds.inData = (const unsigned char*)src.data();
		ds.inLen = src.size();
		ds.outData = NULL;
		ds.outLen = 0;
		// start lzma compression, will call the inputCallback and outputCallback with our datastream and will write to outData
		rc = elzma_compress_run(hand, inputCallback, (void*)&ds, outputCallback, (void*)&ds, NULL, (void*)&ds);

		if (rc != ELZMA_E_OK)
		{
			if (ds.outData != NULL) delete ds.outData;
			elzma_compress_free(&hand);
			return false;
		}
		
		fout.write((char*)&ds.outLen, sizeof(int));
		fout.write((const char*)ds.outData, ds.outLen);

		//cleanup
		delete ds.outData;
		elzma_compress_free(&hand);

		clock_t end = clock();

		float formatedOutput = 0.f;
		float formatedInput = 0.f;
		std::string formatedOutputExt;
		std::string formatedInputExt;
		formatBytes(src.length(), formatedInput, formatedInputExt);
		formatBytes(ds.outLen, formatedOutput, formatedOutputExt);

		OSG_WARN << "[LZMA] From: " << formatedInput << formatedInputExt.c_str() << " to " << formatedOutput << formatedOutputExt.c_str() << " took " << ((float)end-start) / CLOCKS_PER_SEC << " seconds" << std::endl;

		return true;
	}

	virtual bool decompress(std::istream& fin, std::string& target)
	{
		// first we need the length
		unsigned int length;
		fin.read((char*)&length, sizeof(int));

		// we need to copy over our data from the file
		char* buffer = (char*)malloc(length);
		fin.read(buffer, length);
		
		struct dataStream ds;
		ds.inData = (unsigned char*)buffer;
		ds.inLen = length;
		ds.outData = NULL;
		ds.outLen = 0;
		elzma_decompress_handle dehand = elzma_decompress_alloc();
		// run the decompression, similar to how we did the compression.
		int rc = elzma_decompress_run(dehand, inputCallback, (void*)&ds, outputCallback, (void*)&ds, ELZMA_lzip);
		if (rc != ELZMA_E_OK)
		{
			if (ds.outData != NULL) delete ds.outData;
			elzma_decompress_free(&dehand);
			return false;
		}
		// output our data to the target
		target = std::string(ds.outData, ds.outData + ds.outLen);

		//cleanup
		delete ds.outData;
		delete buffer;
		elzma_decompress_free(&dehand);

		return true;
	}
};

REGISTER_COMPRESSOR("lzma", LZMACompressor)