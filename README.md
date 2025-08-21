# alnview - multi image viewer
****alnview**** is a tool for quickly viewing, arranging, aligning, and overlaying multiple images on a canvas. 

![screenshot](demo_img/screenshot.png)

Positioning, zooming, draw order, transparency, opacity, cropping, browsing, and rotation (of both the images and the canvas) are supported, as well as hotkeys to jump between canvas views, saving the current setup, and more.

In particular, you can zoom and rotate images about any arbitrary point, which makes alignment of images easy, and is something that seems to be missing from similar tools.

![demo](demo_img/demo.gif)

It also makes a good document viewer, much better than any PDF reader especially for scanned manuals which have schematics or other diagrams spanning multiple pages (this was the original use case I had in mind).

![demo](demo_img/demo2.gif)

It supports quite extreme levels of zoom, and is great for just messing around.

![demo](demo_img/demo3.gif)

### What's the catch?
All loaded images are stored uncompressed in RAM, which makes it very fast, but also very RAM heavy. On Windows, loading 101 PNGs of document scans at 300 DPI totaling 107MB, it takes more than 6GB of RAM. Even loading a bunch of BMPs comes in at about double the size they are on disk. It may be worth it to manually downscale your images first before loading them. But, this is why you have 64GB ram in your PC, right?

You might find this program similar to [Feh](https://github.com/derf/feh), [PureRef](https://www.pureref.com/), and [BeeRef](https://beeref.org/), if not Google Earth and other similar mapping tools.


Alnview is an [SDL2](https://www.libsdl.org/), so it can load all image file types that ```IMG_Load()``` knows about. This is at least PNG, JPG, WebP, BMP, TIFF, and GIF.


The included font.ttf is one of the [dejavu fonts included with Linux Mint], but any should work.