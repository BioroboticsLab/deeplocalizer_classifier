
#include <QCoreApplication>
#include <boost/program_options.hpp>
#include <chrono>
#include <functional>
#include <ManuallyTagger.h>

#include "ProposalGenerator.h"
#include "qt_helper.h"
#include "utils.h"

using namespace deeplocalizer;
using namespace std::chrono;
using boost::optional;
using boost::make_optional;
namespace po = boost::program_options;
namespace io = boost::filesystem;

po::options_description desc_option("Options");
po::positional_options_description positional_opt;

time_point<system_clock> start_time;

void setupOptions() {
    desc_option.add_options()
            ("help,h", "Print help messages")
            ("pathfile", po::value<std::vector<std::string>>(), "File with image paths");

    positional_opt.add("pathfile", 1);
}

std::vector<ImageDesc> getImagesDone(std::vector<ImageDesc> & images_todo) {
    std::vector<ImageDesc> images_done;
    for(auto & desc : images_todo) {
        desc.setSavePathExtension(ProposalGenerator::IMAGE_DESC_EXT);
        if(io::exists(desc.savePath())) {
            images_done.emplace_back(*ImageDesc::load(desc.savePath()));
        }
    }
    images_todo.erase(
            std::remove_if(
                    images_todo.begin(), images_todo.end(),
                    [&](auto & desc) {
                        return io::exists(desc.savePath());
                    }),
            images_todo.end()
    );
    if (images_done.size() > 0) {
        std::cout << "Found " << images_done.size() << " image description files." << std::endl;
        std::cout << std::endl;
    }
    return images_done;
}

int run(QCoreApplication & qapp,
        std::string pathfile) {
    auto images_todo = ImageDesc::fromPathFile(pathfile);
    auto images_done = getImagesDone(images_todo);
    if (images_todo.empty()) {
        std::cout << "Nothing to do, images are allready processed" << std::endl;
        return 0;
    }
    auto gen = new ProposalGenerator(images_todo, images_done);
    start_time = system_clock::now();
    auto printProgressFn = std::bind(&printProgress<system_clock>, std::cref(start_time), std::placeholders::_1);
    gen->connect(gen, &ProposalGenerator::progress, printProgressFn);
    gen->connect(gen, &ProposalGenerator::finished,
                &qapp, &QCoreApplication::quit, Qt::QueuedConnection);

    printProgress(start_time, 0);
    gen->processPipeline();
    return qapp.exec();
}
void printUsage() {
    std::cout << "Usage: generate_proposals [options] pathfile.txt "<< std::endl;
    std::cout << "    where pathfile.txt contains paths to images."<< std::endl;
    std::cout << desc_option << std::endl;
}
int main(int argc, char* argv[])
{
    QCoreApplication qapp(argc, argv);
    deeplocalizer::registerQMetaTypes();
    setupOptions();
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc_option)
                      .positional(positional_opt).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        printUsage();
        return 0;
    }
    if(vm.count("pathfile")) {
        auto pathfile = vm.at("pathfile").as<std::vector<std::string>>().at(0);
        return run(qapp, pathfile);
    } else {
        std::cout << "No pathfile or output directory given." << std::endl;
        printUsage();
        return 1;
    }
}