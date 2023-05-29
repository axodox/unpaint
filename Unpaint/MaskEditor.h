#pragma once
#include "pch.h"
#include "MaskEditor.g.h"

namespace winrt::Unpaint::implementation
{
  struct MaskEditor : MaskEditorT<MaskEditor>
  {
    enum class MaskTool
    {
      Brush,
      Eraser
    };

  public:
    MaskEditor();

    Windows::Graphics::Imaging::BitmapSize MaskResolution();
    void MaskResolution(Windows::Graphics::Imaging::BitmapSize value);
    static Windows::UI::Xaml::DependencyProperty MaskResolutionProperty();

    double BrushSize();
    void BrushSize(double value);

    double BrushEdge();
    void BrushEdge(double value);

    bool IsBrushSelected();
    bool IsEraserSelected();

    void SelectBrush();
    void SelectEraser();

    void ClearMask();

    bool IsUndoAvailable();
    void UndoAction();
    
    bool IsRedoAvailable();
    void RedoAction();

    void OnCanvasPointerPressed(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& eventArgs);
    void OnCanvasPointerMoved(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& eventArgs);
    void OnCanvasPointerReleased(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& eventArgs);
    void OnCanvasPointerExited(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& eventArgs);
    void OnCanvasPointerWheelChanged(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& eventArgs);

    void OnCanvasLoading(Windows::Foundation::IInspectable const& sender, Windows::Foundation::IInspectable const& eventArgs);
    void OnCanvasDraw(Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl const& sender, Microsoft::Graphics::Canvas::UI::Xaml::CanvasDrawEventArgs const& eventArgs);

    event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value);
    void PropertyChanged(event_token const& token);

  private:
    static Windows::UI::Xaml::DependencyProperty _maskResolutionProperty;

    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;
    Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl _canvasControl;

    float _brushSize;
    float _brushEdge;

    MaskTool _selectedTool;

    Microsoft::Graphics::Canvas::CanvasDevice _canvasDevice;
    Microsoft::Graphics::Canvas::CanvasBitmap _checkerboardBitmap;
    Microsoft::Graphics::Canvas::CanvasRenderTarget _maskTarget;
    Microsoft::Graphics::Canvas::Brushes::CanvasSolidColorBrush _maskBrush;
    Microsoft::Graphics::Canvas::Brushes::CanvasImageBrush _opacityBrush;

    bool _isPenDown;
    std::optional<Windows::Foundation::Point> _currentPosition, _previousPosition;

    int32_t _maskHistoryPosition;
    std::vector<com_array<uint8_t>> _maskHistory;

    fire_and_forget LoadResourcesAsync();
    void EnsureMaskTarget();
    void UpdateHistory();
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct MaskEditor : MaskEditorT<MaskEditor, implementation::MaskEditor>
  {
  };
}
