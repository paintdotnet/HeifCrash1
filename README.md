# HeifCrash1
Minimal repro for heap corruption crash in MSFT's HEIF WIC codec

arg1 is the input file name, arg2 is the output filename

If arg1 is the included `bug1926 Scen3 Update.jpg`, then you'll get a heap corruption and process death once the `IWICBitmapEncoder` is released.

The crash doesn't happen if the `ImageQuality` parameter is set to 0.94f or below. Definitely happens at 0.95f. I haven't exhaustively tested all values.

Also, even when `ImageQuality` is 0.94f, the output image doesn't match the input image. It's been "squished" a bit horizontally. Something's definitely fishy here.
