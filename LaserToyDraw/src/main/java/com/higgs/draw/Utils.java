package com.higgs.draw;

import org.apache.batik.anim.dom.SAXSVGDocumentFactory;
import org.apache.batik.anim.dom.SVGOMPathElement;
import org.apache.batik.dom.svg.SVGItem;
import org.apache.batik.util.XMLResourceDescriptor;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.svg.SVGPathSegList;

import java.io.File;
import java.io.IOException;

public class Utils {
    public static final String EMPTY_STRING = "";

    private static final String PATH_ELEMENT_NAME = "path";

    public static boolean isEmpty(final String string) {
        return string == null || string.isEmpty();
    }

    public static String svgToStringArr(final File file) throws IOException {
        return svgToStringArr(file.getAbsolutePath());
    }

    public static String svgToStringArr(final String uri) throws IOException {
        if(!isEmpty(uri)) {
            return svgToStringArr(new SAXSVGDocumentFactory(XMLResourceDescriptor.getXMLParserClassName()).createDocument(uri));
        }
        return EMPTY_STRING;
    }

    public static String svgToStringArr(final Document svg) {
        final StringBuilder result = new StringBuilder();
        if(svg != null) {
            final NodeList pathNodes = svg.getDocumentElement().getElementsByTagName(PATH_ELEMENT_NAME);
            for(int i = 0; i < pathNodes.getLength(); i++) {
                result.append(new MetaPostPath(pathNodes.item(i)).toCode()).append("\n");
            }
        }
        return result.toString();
    }

    private static class MetaPostPath {
        private SVGOMPathElement _pathElement;

        MetaPostPath(final Node pathNode) {
            _pathElement = (SVGOMPathElement)pathNode;
        }

        String toCode() {
            final StringBuilder sb = new StringBuilder(16384);
            final SVGOMPathElement pathElement = _pathElement;
            final SVGPathSegList pathList = pathElement.getNormalizedPathSegList();

            for(int i = 0; i < pathList.getNumberOfItems(); i++) {
                final SVGItem item = (SVGItem)pathList.getItem(i);
                if(item != null) {
                    sb.append(String.format("%s%n", item.getValueAsString()));
                }
            }
            return sb.toString();
        }
    }
}
