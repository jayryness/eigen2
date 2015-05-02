#include <windows.h>
#include <wrl/client.h>
#include <cstdio>
#include "render/Renderer.h"

class Demo
{
public:

    Demo() : _hwnd(0), _quit(false) {}

    void run();

private:

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void init();

    LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam);

    HWND _hwnd;
    bool _quit;
};

LRESULT CALLBACK Demo::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CREATE)
    {
        Demo* demo = (Demo*)((CREATESTRUCT*)lParam)->lpCreateParams;

        demo->_hwnd = hwnd;
        SetWindowLongPtr(hwnd, 0, (LONG_PTR)demo);
    }

    Demo* demo = (Demo*)GetWindowLongPtr(hwnd, 0);
    if (demo)
    {
        return demo->wndProc(message, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

void Demo::init()
{
    WNDCLASS wc = { sizeof(WNDCLASSEX) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(LONG_PTR);
    wc.hInstance = GetModuleHandle(NULL);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"Eigen";
    ATOM atom = RegisterClass(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, L"Eigen Demo", WS_OVERLAPPEDWINDOW, 0, 0, 600, 600, NULL, NULL, wc.hInstance, this);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

LRESULT Demo::wndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CREATE || message == WM_NCCREATE)
    {
        return 1;
    }

    LRESULT result = 0;

    switch (message)
    {
    case WM_SIZE:
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
        }
        return 0;

    case WM_DISPLAYCHANGE:
        InvalidateRect(_hwnd, NULL, FALSE);
        return 0;

    case WM_PAINT:
        ValidateRect(_hwnd, NULL);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        _quit = true;
        return 1;

    case WM_MOUSEMOVE:
        {
            float x = 0.f;
            float y = 0.f;
            //x -= float(g_mx) / pDemoApp->m_pRenderTarget->GetSize().width;
            //y -= float(g_my) / pDemoApp->m_pRenderTarget->GetSize().height;

            //g_mx = LOWORD(lParam);
            //g_my = HIWORD(lParam);

            //x += float(g_mx) / pDemoApp->m_pRenderTarget->GetSize().width;
            //y += float(g_my) / pDemoApp->m_pRenderTarget->GetSize().height;

            //g_dx += x*100.f;
            //g_dy += y*100.f;

            //float norm = sqrt(g_dx*g_dx + g_dy*g_dy);
            //g_dx /= norm;
            //g_dy /= norm;

            InvalidateRect(_hwnd, NULL, FALSE);
        }
        return 0;
    }

    return DefWindowProc(_hwnd, message, wParam, lParam);
}

void Demo::run()
{
    printf("Initializing...");

    init();
    RECT clientRect;
    GetClientRect(_hwnd, &clientRect);

    eigen::Renderer::Config renderConfig;
    renderConfig.debugEnabled       = true;
    renderConfig.scratchSize        = 16*1024*1024;
    renderConfig.submissionThreads  = 1;

    eigen::Renderer renderer;
    {
        eigen::Error error = renderer.initialize(renderConfig);
        if (Failed(error))
        {
            puts(error.getText());
            MessageBoxA(_hwnd, error.getText(), "Error", MB_OK);
            return;
        }
        eigen::DisplayPtr display = renderer.createDisplay();
        display.ptr->bindToWindow(_hwnd);

        eigen::TargetSetPtr displayTargets = renderer.createTargetSet();
        {
            eigen::TargetSet::Config cfg;
            cfg.textures[0] = display.ptr->getTarget();
            displayTargets.ptr->initialize(cfg);
        }

        {
            eigen::RenderBuffer::Config cfg;
            cfg.elementStride = 16;
            cfg.elementCount = 32;
            cfg.bindings |= eigen::RenderBuffer::Bindings::RenderTarget;

            eigen::RenderBufferPtr buffer = renderer.createBuffer();
            buffer.ptr->initialize(cfg);
        }

        const eigen::RenderBin* bin = renderer.getBin("It's a bin");
        const eigen::RenderBin* anotherBin = renderer.getBin("It's another bin");

        eigen::RenderPlanPtr plan = renderer.createPlan();

        eigen::ClearStage& clearStage = plan.ptr->addClearStage(displayTargets.ptr);
        clearStage.flags = eigen::ClearStage::Flags::Color_Depth_Stencil;
        clearStage.colors[0] = eigen::Float4::Xyzw(0.7f, 0.8f, 0.9f, 0.f);
        {
            eigen::BatchStage& batchStage = plan.ptr->addBatchStage(displayTargets.ptr);
            batchStage.attachBin(bin);
        }
        {
            eigen::BatchStage& batchStage = plan.ptr->addBatchStage(displayTargets.ptr);
            batchStage.sortType = eigen::BatchStage::SortType::IncreasingDepth;
            batchStage.attachBin(anotherBin);
        }
        {
            eigen::BatchStage& batchStage = plan.ptr->addBatchStage(displayTargets.ptr);
            batchStage.sortType = eigen::BatchStage::SortType::DecreasingDepth;
            batchStage.attachBin(bin);
            batchStage.attachBin(anotherBin);
        }
        error = plan.ptr->validate();
        if (Failed(error))
        {
            puts(error.getText());
            MessageBoxA(_hwnd, error.getText(), "Error", MB_OK);
            return;
        }

        printf(" done.\n");

        HWND consoleHwnd = GetConsoleWindow();
        ShowWindow(consoleHwnd, SW_MINIMIZE);

        printf("Running.\n");

        eigen::RenderBatch* batch = nullptr;    // no batch creation API yet!

        while (!_quit)
        {
            MSG msg;
            while (PeekMessage(&msg, _hwnd, 0, 0, PM_REMOVE))
            {
                DispatchMessage(&msg);
            }

            eigen::BatchQueue* batchQ = renderer.openBatchQueue(plan.ptr);

            batchQ->commitBatch(batch, bin,        0.f);
            batchQ->commitBatch(batch, anotherBin, 1.f);
            batchQ->commitBatch(batch, bin,        2.f);
            batchQ->commitBatch(batch, anotherBin, 3.f);

            batchQ->finish();

            renderer.commenceWork();

            Sleep(0);
        }
    }

}

int main(int argc, const char** argv)
{
    int result = 0;
    Demo demo;

    demo.run();

    HWND consoleHwnd = GetConsoleWindow();
    ShowWindow(consoleHwnd, SW_RESTORE);

    printf("Cleaning up.\n\n");

    eigen::Mallocator* mallocator = eigen::Mallocator::Get();
    if (mallocator->getCount())
    {
        printf("Leak detected: %d unfreed allocations!\n", mallocator->getCount());

        DebugBreak();

        result = -1;
    }
    else
    {
        printf("No leaks.\n");
    }

    printf("\n%s exit in ", result == 0 ? "Clean" : "Dirty");
    for (int secs = 3; secs > 0; secs--)
    {
        printf("%d...", secs);
        Sleep(1000);
    }

    return result;
}

