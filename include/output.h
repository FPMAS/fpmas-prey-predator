#ifndef FPMAS_PREY_PREDATOR_OUTPUT_H
#define FPMAS_PREY_PREDATOR_OUTPUT_H

#include "fpmas.h"
#include "prey_predator.h"
#include <fstream>

#define PRINT_CONFIG(CLASS, PARAM) \
	std::cout << "\t- " #CLASS "::" #PARAM ": " << CLASS::PARAM << std::endl;

void print_current_config();

using fpmas::io::DistributedCsvOutput;
using fpmas::io::Local;
using fpmas::io::Reduce;

class FileOutput {
	protected:
		std::ofstream output_file;
		FileOutput(std::string filename)
			: output_file(filename) {}
};

class ModelOutput :
	public FileOutput,
	public DistributedCsvOutput<
	Local<fpmas::api::runtime::Date>, Reduce<std::size_t>, Reduce<std::size_t>, Reduce<std::size_t>> {
		public:
			ModelOutput(fpmas::api::model::Model& model);
	};

class GraphOutput :
	public FileOutput,
	public DistributedCsvOutput<
	Local<fpmas::api::runtime::Date>, Local<std::size_t>, Local<std::size_t>, Reduce<std::size_t>, Reduce<std::size_t>> {
		public:
			GraphOutput(fpmas::api::model::Model& model);
	};

#endif
