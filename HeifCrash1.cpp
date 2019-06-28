
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

#include <Windows.h>

#include <wincodec.h>

// ATL/COM
#include <comcat.h>
#define _ATL_FREE_THREADED
#include <atlbase.h>
#include <atlcoll.h>
#include <atlcom.h>
#include <atlcomcli.h>
#include <atlenc.h>

CComModule _Module;

// WIC
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")

#include <iostream>
using namespace std;

#define IFS(hr, expr) \
    cout << #expr; \
    if (SUCCEEDED((hr))) \
    { \
        (hr) = (expr); \
    } \
    cout << " -> " << (hr) << endl; \

// Creates an instance of T and returns it to the caller with
// a ref count of 1. It will still need to be initialized.
template<typename T>
static HRESULT CreateComObject(T** ppResult)
{
    if (ppResult == NULL)
    {
        return E_POINTER;
    }

    *ppResult = NULL;

    HRESULT hr = S_OK;

    CComObject<T>* pObject = NULL;
    if (SUCCEEDED(hr))
    {
        hr = CComObject<T>::CreateInstance(&pObject);
    }

    if (SUCCEEDED(hr))
    {
        pObject->AddRef();
        *ppResult = pObject;
    }

    return hr;
}

int wmain(int argc, wchar_t** argv)
{
    if (argc != 3)
    {
        cout << "arg1 is the input filename (any type), and arg2 is the output filename (using HEIF/HEIC)" << endl;
        cout << "NOTE: output file must not exist!!!" << endl;
        return 1;
    }

    const wchar_t* szInputFileName = argv[1];
    cout << "Input file: " << szInputFileName << endl;

    const wchar_t* szOutputFileName = argv[2];
    cout << "Output file: " << szOutputFileName << endl;

    HRESULT hr = S_OK;

    IFS(hr, CoInitializeEx(NULL, COINIT_MULTITHREADED));

    CComPtr<IWICImagingFactory2> spFactory;
    IFS(hr, CoCreateInstance(
        CLSID_WICImagingFactory2,
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(IWICImagingFactory),
        reinterpret_cast<void**>(&spFactory)));

    CComPtr<IWICStream> spInputStream;
    IFS(hr, spFactory->CreateStream(&spInputStream));

    IFS(hr, spInputStream->InitializeFromFilename(
        szInputFileName,
        GENERIC_READ));

    CComPtr<IWICBitmapDecoder> spDecoder;
    IFS(hr, spFactory->CreateDecoderFromStream(
        spInputStream,
        NULL,
        WICDecodeMetadataCacheOnDemand,
        &spDecoder));

    CComPtr<IWICBitmapFrameDecode> spFrame0Decode;
    IFS(hr, spDecoder->GetFrame(
        0,
        &spFrame0Decode));

    CComPtr<IWICStream> spOutputStream;
    IFS(hr, spFactory->CreateStream(&spOutputStream));

    IFS(hr, spOutputStream->InitializeFromFilename(
        szOutputFileName,
        GENERIC_READ | GENERIC_WRITE));

    CComPtr<IWICBitmapEncoder> spEncoder;
    IFS(hr, spFactory->CreateEncoder(
        GUID_ContainerFormatHeif,
        &GUID_VendorMicrosoft,
        &spEncoder));

    IFS(hr, spEncoder->Initialize(
        spOutputStream,
        WICBitmapEncoderNoCache));

    CComPtr<IWICBitmapFrameEncode> spFrame0Encode;
    CComPtr<IPropertyBag2> spFrameOptions;
    IFS(hr, spEncoder->CreateNewFrame(
        &spFrame0Encode,
        &spFrameOptions));

    CComBSTR bstrOptionName = SysAllocString(L"ImageQuality");    

    // Using a quality of 0.95 causes a crash. Using 0.94 or below (I tested as far down as 0.50) works fine.
    if (SUCCEEDED(hr))
    {
        PROPBAG2 option = { 0 };
        option.pstrName = bstrOptionName;

        VARIANT varValue;
        VariantInit(&varValue);
        varValue.vt = VT_R4;
        varValue.fltVal = 0.94f;

        IFS(hr, spFrameOptions->Write(
            1,
            &option,
            &varValue));
    }

    IFS(hr, spFrame0Encode->Initialize(spFrameOptions));

    IFS(hr, spFrame0Encode->WriteSource(
        spFrame0Decode,
        NULL));

    IFS(hr, spFrame0Encode->Commit());
    if (SUCCEEDED(hr)) 
    {
        spFrame0Encode.Release();
    }

    IFS(hr, spEncoder->Commit());
    if (SUCCEEDED(hr))
    {
        spEncoder.Release();
    }
    
    cout << "Return value, HRESULT = " << hr << endl;
    return hr;
}
