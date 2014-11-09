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

    {
        eigen::BitSet<128> mask;
        unsigned start, end;
        mask.getRange(start, end);
        unsigned hash = mask.hash() & 0xff;
        mask.complement();
        hash = mask.hash() & 0xff;
        mask.getRange(start, end);
        mask.set(0, false);
        hash = mask.hash() & 0xff;
        mask.getRange(start, end);
        mask >>= 1;
        hash = mask.hash() & 0xff;
        mask.getRange(start, end);
        mask.complement();
        hash = mask.hash() & 0xff;
        mask.getRange(start, end);
        mask.set(0, true);
        hash = mask.hash() & 0xff;
        mask.set(1, true);
        hash = mask.hash() & 0xff;
        mask.set(2, true);
        hash = mask.hash() & 0xff;
        mask.set(3, true);
        hash = mask.hash() & 0xff;
        mask <<= 27;
        hash = mask.hash() & 0xff;
        mask.getRange(start, end);
        mask >>= 4;
        hash = mask.hash() & 0xff;
        mask.getRange(start, end);
        mask.clear();
    }

    init();
    RECT clientRect;
    GetClientRect(_hwnd, &clientRect);

    eigen::Renderer::Config renderConfig;
    renderConfig.debugEnabled   = true;
    renderConfig.scratchSize    = 16*1024*1024;
    renderConfig.submitThreads  = 1;

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

        //Eigen::Display* display = system.GetDisplaySystem().NextDisplay();

        //Eigen::Renderer& renderer = system.GetRenderer();
        const eigen::RenderPort* port = renderer.getPort("One");
        renderer.getPort("One");
        renderer.getPort("Two");
        renderer.getPort("Three");
        port = renderer.getPort("Four");

        eigen::RenderPlanPtr plan = renderer.createPlan();

        eigen::ClearStage& clearStage = plan.ptr->addClearStage(displayTargets.ptr);
        clearStage.flags = eigen::ClearStage::Flags::Color_Depth_Stencil;
        {
            eigen::BatchStage& batchStage = plan.ptr->addBatchStage(displayTargets.ptr);
            batchStage.addPort(port);
        }
        {
            eigen::BatchStage& batchStage = plan.ptr->addBatchStage(displayTargets.ptr);
            batchStage.sortType = eigen::BatchStage::SortType::IncreasingDepth;
            batchStage.addPort(port);
        }
        {
            eigen::BatchStage& batchStage = plan.ptr->addBatchStage(displayTargets.ptr);
            batchStage.sortType = eigen::BatchStage::SortType::DecreasingDepth;
            batchStage.addPort(port);
            batchStage.addPort(renderer.getPort("Foo"));
        }
        error = plan.ptr->validate();
        if (Failed(error))
        {
            puts(error.getText());
            MessageBoxA(_hwnd, error.getText(), "Error", MB_OK);
            return;
        }

        //eigen::PipelinePtr pipeline = renderer.createPipeline();
        //pipeline.ptr->initialize(4);

        //eigen::ClearStage clearStage;
        //clearStage.targets = displayTargets.ptr;
        //clearStage.flags = eigen::ClearStage::Flags::Color_Depth_Stencil;
        //pipeline.ptr->addStage(clearStage);

        //Eigen::TextureSystem& textureSystem = system.GetTextureSystem();
        //eigen::TextureTargetPtr::Config targetCfg;
        //targetCfg.setWidth(clientRect.right - clientRect.left);
        //targetCfg.setHeight(clientRect.bottom - clientRect.top);
        //targetCfg.setFormat(eigen::Format::RGB10_A2);
        //eigen::TextureTargetPtr target = renderer.CreateTarget(targetCfg);//display->GetTarget();// 

        //Eigen::TargetGroupPtr testGroup = textureSystem.CreateTargetGroup(target.ptr);

        printf(" done.\n");

        HWND consoleHwnd = GetConsoleWindow();
        ShowWindow(consoleHwnd, SW_MINIMIZE);

        printf("Running.\n");

        while (!_quit)
        {
            MSG msg;
            while (PeekMessage(&msg, _hwnd, 0, 0, PM_REMOVE))
            {
                DispatchMessage(&msg);
            }

            eigen::Worklist* worklist = renderer.openWorklist(plan.ptr);

            worklist->commitBatch(nullptr, port, 0.f);

            worklist->finish();

            renderer.commenceWork();

            //Eigen::Renderer& renderer = system.GetRenderer();
            //Eigen::RenderContext* context = renderer.BeginContext();
            //context->CommitBatch(port, Eigen::RenderBatch(), 0.f);
            //Eigen::Stage::Info stageInfo;
            //stageInfo.type = Eigen::Stage::cType_Clear;
            //stageInfo.clearParams.flags = Eigen::Stage::cClearFlag_Color;
            //stageInfo.clearParams.colors[0][0] = 0.7f;
            //stageInfo.clearParams.colors[0][1] = 0.8f;
            //stageInfo.clearParams.colors[0][2] = 0.9f;
            //stageInfo.clearParams.colors[0][3] = 1.f;
            //stageInfo.targetGroup = testGroup;
            //Eigen::Stage stage(stageInfo);
            //renderer.EndContext(context, &stage, 1);
            //system.SubmitFrame();

            //system.GetDisplaySystem().PresentAll();
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
        printf("---------------------------------------------\n");
        printf("Memory leak detected: %d unfreed allocations!\n", mallocator->getCount());
        printf("---------------------------------------------\n");

        DebugBreak();

        result = -1;
    }
    else
    {
        printf("----------------\n");
        printf("No memory leaks.\n");
        printf("----------------\n");
    }

    printf("\nExiting %s in ", result == 0 ? "cleanly" : "with error code");
    for (int secs = 3; secs > 0; secs--)
    {
        printf("%d...", secs);
        Sleep(1000);
    }

    return result;
}

