#include "HuggingFaceSchema.h"

namespace Axodox::Web
{
  class HuggingFaceClient
  {
  public:
    HuggingFaceClient();

    std::vector<std::string> GetModels();

    std::optional<HuggingFaceModelDetails> GetModel(std::string_view modelId);

  private:
    static const wchar_t* const _baseUri;
    winrt::Windows::Web::Http::HttpClient _httpClient;
    winrt::Windows::Web::Http::Headers::HttpCredentialsHeaderValue _credentialsHeader{ nullptr };

    static winrt::Windows::Web::Http::HttpClient CreateClient();

    winrt::hstring TryQuery(std::wstring_view uri);
  };
}