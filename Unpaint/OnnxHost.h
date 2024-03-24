#pragma once
#include "pch.h"

namespace winrt::Unpaint
{
	class OnnxHost
	{
	public:
		OnnxHost();

		const std::shared_ptr<Axodox::MachineLearning::Sessions::OnnxEnvironment>& Environment() const;
		const std::shared_ptr<Axodox::MachineLearning::Executors::OnnxExecutor>& Executor() const;

		Axodox::MachineLearning::Sessions::OnnxSessionParameters ParametersFromFile(const std::filesystem::path& path) const;
		Axodox::MachineLearning::Sessions::OnnxSessionParameters ParametersFromFile(const winrt::Windows::Storage::StorageFile& file) const;

	private:
		std::shared_ptr<Axodox::MachineLearning::Sessions::OnnxEnvironment> _environment;
		std::shared_ptr<Axodox::MachineLearning::Executors::OnnxExecutor> _executor;
	};
}