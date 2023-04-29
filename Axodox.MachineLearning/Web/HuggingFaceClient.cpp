#include "pch.h"
#include "HuggingFaceClient.h"
#include "Infrastructure/Win32.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Json;
using namespace std;
using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Security::Cryptography;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::Web::Http;
using namespace winrt::Windows::Web::Http::Headers;
using namespace winrt::Windows::Web::Http::Filters;

namespace Axodox::Web
{
  const wchar_t* const HuggingFaceClient::_baseUri = L"https://huggingface.co/api/";

  HuggingFaceClient::HuggingFaceClient() :
    _httpClient(CreateClient())
  {
    hstring credentialString{ L"Bearer : $token" };
    auto encodedCredentialBuffer = CryptographicBuffer::ConvertStringToBinary(credentialString, BinaryStringEncoding::Utf8);
    auto encodedCredentials = CryptographicBuffer::EncodeToBase64String(encodedCredentialBuffer);
    _credentialsHeader = HttpCredentialsHeaderValue(L"Basic", encodedCredentials);
  }

  std::vector<std::string> HuggingFaceClient::GetModels()
  {
    auto result = to_string(TryQuery(L"models?filter=unpaint,stable_diffusion_model"));
    auto models = try_parse_json<vector<HuggingFaceModelInfo>>(result);
    if (!models) return {};

    vector<string> results;
    results.reserve(models->size());
    for (auto& model : *models)
    {
      results.push_back(*model.Id);      
    }

    return results;
  }

  std::optional<HuggingFaceModelDetails> HuggingFaceClient::GetModel(std::string_view modelId)
  {
    auto result = to_string(TryQuery(to_wstring("models/" + string(modelId))));
    return try_parse_json<HuggingFaceModelDetails>(result);
  }

  winrt::Windows::Web::Http::HttpClient HuggingFaceClient::CreateClient()
  {
    HttpBaseProtocolFilter filter{};

    auto cacheControl = filter.CacheControl();
    cacheControl.ReadBehavior(HttpCacheReadBehavior::MostRecent);
    cacheControl.WriteBehavior(HttpCacheWriteBehavior::NoCache);

    return HttpClient{ filter };
  }
  
  winrt::hstring HuggingFaceClient::TryQuery(std::wstring_view uri)
  {
    hstring result;

    try
    {
      //Create request
      HttpRequestMessage request{};
      {
        request.RequestUri(Uri{ _baseUri + wstring(uri) });
        request.Method(HttpMethod::Get());

        const auto& headers = request.Headers();
        headers.Accept().TryParseAdd(L"application/json");
        //headers.Authorization(_credentialsHeader);
      }

      //Execute
      auto requestResult = _httpClient.TrySendRequestAsync(request).get();
      if (!requestResult.Succeeded()) return L"";

      //Parse response
      auto response = requestResult.ResponseMessage();
      if (response.StatusCode() != HttpStatusCode::Ok) return L"";

      result = response.Content().ReadAsStringAsync().get();
    }
    catch (...)
    {
      return L"";
    }

    return result;
  }
}