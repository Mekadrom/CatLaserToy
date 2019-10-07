package com.higgs.draw;

import com.higgs.draw.io.BufferedImageTranscoder;
import com.higgs.draw.io.ImageInputStreamAdaptor;

import javax.imageio.ImageTranscoder;
import javax.imageio.stream.ImageInputStream;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

public class Utils {
    private  BufferedImage svgToBufferedImage(ImageInputStream inputStream)
            throws IOException {

        // The input to an ImageReader is an ImageInputStream but
        // the TranscoderInput constructor expects a standard ouput stream,
        // which is not compatible with ImageInputStream.  A work around to
        // this problem is to use a Wrapper class to encapsulate the
        // ImageInputStream in a class that extends InputStream.
        ImageInputStreamAdaptor imageInputStreamAdaptor =
                new ImageInputStreamAdaptor(inputStream);

        // create a servlet
        ImageTranscoder bufferedImgTranscoder =
                new BufferedImageTranscoder();

        TranscoderInput transcoderInput =
                new TranscoderInput(imageInputStreamAdaptor);

        // create the servlet output
        BufferedImageTranscoderOutput output =
                new BufferedImageTranscoderOutput();

        // perform the transcoding
        try {
            bufferedImgTranscoder.transcode(transcoderInput, output);
        } catch (TranscoderException e) {
            String imagePath = transcoderInput.getURI();
            logger.error("svg-transcoding-error" + imagePath, e);
            throw new IOException(e.getLocalizedMessage());
        }
        return output.getBufferedImage();
    }
}
