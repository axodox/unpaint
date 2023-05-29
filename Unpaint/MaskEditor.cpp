#include "pch.h"
#include "MaskEditor.h"
#include "MaskEditor.g.cpp"
#include "Storage/FileIO.h"

using namespace Axodox::Storage;
using namespace std;
using namespace winrt;
using namespace winrt::Microsoft::Graphics::Canvas;
using namespace winrt::Microsoft::Graphics::Canvas::Brushes;
using namespace winrt::Microsoft::Graphics::Canvas::Effects;
using namespace winrt::Microsoft::Graphics::Canvas::UI::Xaml;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Graphics::DirectX;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Unpaint::implementation
{
  Windows::UI::Xaml::DependencyProperty MaskEditor::_maskResolutionProperty = DependencyProperty::Register(
    L"MaskResolution",
    xaml_typename<BitmapSize>(),
    xaml_typename<Unpaint::MaskEditor>(),
    PropertyMetadata{ box_value(BitmapSize{768, 768}) }
  );

  MaskEditor::MaskEditor() :
    _canvasDevice(CanvasDevice::GetSharedDevice()),
    _maskBrush(_canvasDevice, Colors::Blue()),
    _opacityBrush(nullptr),
    _checkerboardBitmap(nullptr),
    _brushSize(32.f),
    _brushEdge(8.f),
    _selectedTool(MaskTool::Brush),
    _maskTarget(nullptr),
    _isPenDown(false),
    _currentPosition({}),
    _previousPosition({})
  { 
    LoadResourcesAsync();
  }

  Windows::Graphics::Imaging::BitmapSize MaskEditor::MaskResolution()
  {
    return unbox_value<BitmapSize>(GetValue(_maskResolutionProperty));
  }

  void MaskEditor::MaskResolution(Windows::Graphics::Imaging::BitmapSize value)
  {
    SetValue(_maskResolutionProperty, box_value(value));
  }

  Windows::UI::Xaml::DependencyProperty MaskEditor::MaskResolutionProperty()
  {
    return _maskResolutionProperty;
  }

  double MaskEditor::BrushSize()
  {
    return _brushSize;
  }

  void MaskEditor::BrushSize(double value)
  {
    if (value == _brushSize) return;

    _brushSize = float(value);
    _propertyChanged(*this, PropertyChangedEventArgs(L"BrushSize"));
  }

  double MaskEditor::BrushEdge()
  {
    return _brushEdge;
  }

  void MaskEditor::BrushEdge(double value)
  {
    if (value == _brushEdge) return;

    _brushEdge = float(value);
    _propertyChanged(*this, PropertyChangedEventArgs(L"BrushEdge"));

    _canvasControl.Invalidate();
  }

  bool MaskEditor::IsBrushSelected()
  {
    return _selectedTool == MaskTool::Brush;
  }

  bool MaskEditor::IsEraserSelected()
  {
    return _selectedTool == MaskTool::Eraser;
  }

  void MaskEditor::SelectBrush()
  {
    if (_selectedTool == MaskTool::Brush) return;

    _selectedTool = MaskTool::Brush;
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsBrushSelected"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsEraserSelected"));
  }

  void MaskEditor::SelectEraser()
  {
    if (_selectedTool == MaskTool::Eraser) return;

    _selectedTool = MaskTool::Eraser;
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsBrushSelected"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsEraserSelected"));
  }

  void MaskEditor::ClearMask()
  {
    auto session = _maskTarget.CreateDrawingSession();
    session.Clear(Colors::Transparent());
    _canvasControl.Invalidate();
  }

  bool MaskEditor::IsUndoAvailable()
  {
    return false;
  }

  void MaskEditor::UndoAction()
  {
  }

  bool MaskEditor::IsRedoAvailable()
  {
    return false;
  }

  void MaskEditor::RedoAction()
  {
  }

  void MaskEditor::OnCanvasPointerPressed(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& eventArgs)
  {
    auto uiElement = sender.as<UIElement>();
    auto pointer = eventArgs.GetCurrentPoint(uiElement);
    auto pointerProperties = pointer.Properties();
    
    //Switch tools on right click
    if (pointerProperties.IsRightButtonPressed())
    {
      switch (_selectedTool)
      {
      case MaskTool::Brush:
        SelectEraser();
        break;
      case MaskTool::Eraser:
        SelectBrush();
        break;
      }
    }

    //If left button is pressed and we can capture the pointer start drawing
    if (!pointerProperties.IsLeftButtonPressed() || !uiElement.CapturePointer(eventArgs.Pointer())) return;
    _isPenDown = true;

    //Store position and update canvas
    _currentPosition = pointer.Position();
    _canvasControl.Invalidate();
  }

  void MaskEditor::OnCanvasPointerMoved(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& eventArgs)
  {
    //Update position and canvas
    auto uiElement = sender.as<UIElement>();
    _currentPosition = eventArgs.GetCurrentPoint(uiElement).Position();

    _canvasControl.Invalidate();
  }

  void MaskEditor::OnCanvasPointerReleased(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& eventArgs)
  {
    //Release pen
    auto uiElement = sender.as<UIElement>();
    uiElement.ReleasePointerCapture(eventArgs.Pointer());
    _isPenDown = false;

    //Update position and canvas
    _currentPosition = eventArgs.GetCurrentPoint(uiElement).Position();
    _canvasControl.Invalidate();
  }

  void MaskEditor::OnCanvasPointerExited(Windows::Foundation::IInspectable const& /*sender*/, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& /*eventArgs*/)
  {
    _currentPosition = nullopt;
    _canvasControl.Invalidate();
  }

  void MaskEditor::OnCanvasPointerWheelChanged(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& eventArgs)
  {
    auto uiElement = sender.as<UIElement>();
    auto wheelDelta = eventArgs.GetCurrentPoint(uiElement).Properties().MouseWheelDelta();

    BrushSize(clamp(_brushSize * (1.f + wheelDelta / 1000.f), 1.f, 128.f));
    _canvasControl.Invalidate();
  }

  void MaskEditor::OnCanvasLoading(Windows::Foundation::IInspectable const& sender, Windows::Foundation::IInspectable const& /*eventArgs*/)
  {
    _canvasControl = sender.as<CanvasControl>();
  }

  void MaskEditor::OnCanvasDraw(Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl const& /*sender*/, Microsoft::Graphics::Canvas::UI::Xaml::CanvasDrawEventArgs const& eventArgs)
  {
    EnsureMaskTarget();

    //Update mask
    if (_isPenDown)
    {
      auto session = _maskTarget.CreateDrawingSession();
      
      //Select color and blend mode
      Color brushColor;
      if (_selectedTool == MaskTool::Brush)
      {
        session.Blend(CanvasBlend::Copy);
        brushColor = Colors::White();
      }
      else
      {
        session.Blend(CanvasBlend::Copy);
        brushColor = Colors::Transparent();
      }

      //Define tool path
      vector<Point> positions;
      if (_previousPosition)
      {
        auto distance = sqrt(pow(_currentPosition->X - _previousPosition->X, 2.f) + pow(_currentPosition->Y - _previousPosition->Y, 2.f));
        auto stepCount = max(1, int32_t(ceil(distance / 2.f)));
        for (auto i = 0; i <= stepCount; i++)
        {
          auto t = float(i) / stepCount;
          positions.push_back({
            lerp(_previousPosition->X, _currentPosition->X, t),
            lerp(_previousPosition->Y, _currentPosition->Y, t)
            });
        }
      }
      else
      {
        positions.push_back(*_currentPosition);
      }

      //Draw path
      for (auto point : positions)
      {
        session.FillCircle(point, _brushSize, brushColor);
      }
    }

    _previousPosition = _currentPosition;

    //Update canvas
    {
      //Clear canvas
      const auto& session = eventArgs.DrawingSession();
      session.Blend(CanvasBlend::SourceOver);
      
      //Draw mask
      {
        ICanvasEffect sourceEffect;
        if (_checkerboardBitmap)
        {
          auto tileSize = _checkerboardBitmap.SizeInPixels();

          TileEffect tileEffect;
          tileEffect.Source(_checkerboardBitmap);
          tileEffect.SourceRectangle(Rect{ 0.f, 0.f, float(tileSize.Width), float(tileSize.Height) });
          sourceEffect = tileEffect;
        }
        else
        {
          ColorSourceEffect colorSourceEffect;
          colorSourceEffect.Color(Colors::White());
          sourceEffect = colorSourceEffect;
        }

        GaussianBlurEffect blurEffect;
        blurEffect.BlurAmount(_brushEdge);
        blurEffect.Source(_maskTarget);

        AlphaMaskEffect maskEffect{};
        maskEffect.Source(sourceEffect);
        maskEffect.AlphaMask(blurEffect);
        session.DrawImage(maskEffect);
      }

      //Draw target area
      if (_currentPosition)
      {
        auto brushColor = _selectedTool == MaskTool::Brush ? Colors::Green() : Colors::Orange();
        brushColor.A = 127;
        session.FillCircle(*_currentPosition, _brushSize, brushColor);
      }
    }   
  }

  event_token MaskEditor::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value)
  {
    return _propertyChanged.add(value);
  }

  void MaskEditor::PropertyChanged(event_token const& token)
  {
    _propertyChanged.remove(token);
  }

  fire_and_forget MaskEditor::LoadResourcesAsync()
  {
    auto lifetime = get_strong();

    _checkerboardBitmap = co_await CanvasBitmap::LoadAsync(_canvasDevice, (app_folder() / L"Assets/checkerboard.png").c_str());
    if (_canvasControl) _canvasControl.Invalidate();
  }

  void MaskEditor::EnsureMaskTarget()
  {
    auto resolution = MaskResolution();
    if (_maskTarget && _maskTarget.SizeInPixels() == resolution) return;

    _maskTarget = CanvasRenderTarget(_canvasDevice, float(resolution.Width), float(resolution.Height), 96.f, DirectXPixelFormat::A8UIntNormalized, CanvasAlphaMode::Straight);
    _opacityBrush = CanvasImageBrush(_canvasDevice, _maskTarget);
  }
}
