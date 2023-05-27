#include "pch.h"
#include "UwpStorage.h"

using namespace std;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

namespace Axodox::Storage
{
  std::vector<uint8_t> read_file(const winrt::Windows::Storage::StorageFile& file)
  {
    auto stream = file.OpenReadAsync().get();
    auto buffer = stream.ReadAsync(Buffer{ uint32_t(stream.Size()) }, uint32_t(stream.Size()), InputStreamOptions::None).get();

    vector<uint8_t> result;
    result.resize(buffer.Length());
    memcpy(result.data(), buffer.data(), result.size());

    return result;
  }
}
