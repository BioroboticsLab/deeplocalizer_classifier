#ifndef LOCALIZER_CAFFE_MANUELLCLASSIFIER_H
#define LOCALIZER_CAFFE_MANUELLCLASSIFIER_H


#include <string>
#include <deque>
#include <memory>

#include <boost/version.hpp>
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/basic_archive.hpp>
#include <boost/optional/optional.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <opencv2/core/core.hpp>

#include <QString>
#include <QObject>

#include <pipeline/datastructure/Tag.h>
#include <pipeline/Preprocessor.h>
#include <pipeline/Localizer.h>
#include <pipeline/EllipseFitter.h>

#include "Tag.h"
#include "Image.h"
#include "serialization.h"


namespace deeplocalizer {



class ManuallyTagger : public QObject {
    Q_OBJECT

public slots:
    void save() const;
    void save(const std::string & path) const;
    void loadNextImage();
    void loadLastImage();
    void loadImage(unsigned long idx);
    void loadCurrentImage();
    void doneTagging();
    void doneTagging(unsigned long idx);
signals:
    void loadedImage(unsigned long idx, ImageDescPtr desc, ImagePtr img);
    void outOfRange(unsigned long idx);
    void lastImage();
    void firstImage();
    void progress(double progress);
public:
    static const std::string IMAGE_DESC_EXT;
    static const std::string DEFAULT_SAVE_PATH;

    explicit ManuallyTagger();
    explicit ManuallyTagger(const std::vector<ImageDesc> & descriptions,
                            const std::string & save_path = DEFAULT_SAVE_PATH);
    explicit ManuallyTagger(const std::vector<ImageDescPtr> & descriptions,
                            const std::string & save_path = DEFAULT_SAVE_PATH);
    explicit ManuallyTagger(std::vector<ImageDescPtr> && descriptions,
                            const std::string & save_path = DEFAULT_SAVE_PATH);
    void init();

    static std::unique_ptr<ManuallyTagger> load(const std::string & path);

    const std::string & savePath() const {
        return _save_path;
    }
    void setSavePath(const std::string &save_path) {
        ManuallyTagger::_save_path = save_path;
    }

    const std::vector<ImageDescPtr> &getImageDescs() const {
        return _image_descs;
    }
    bool isDone(unsigned long idx) const {
        // either *.tagger.desc file exists or it is marked as done
        return (idx < _image_descs.size() &&
                boost::filesystem::exists(_image_descs.at(idx)->savePath())) ||
                (idx < _done_tagging.size() && _done_tagging.at(idx));
    }
    bool isDone(const ImageDesc & desc) const {
        return boost::filesystem::exists(desc.savePath());
    }

    unsigned long getIdx() const {
        return _image_idx;
    }
private:
    std::vector<ImageDescPtr> _image_descs;
    std::vector<bool> _done_tagging;
    unsigned long _n_done = 0;
    bool _loaded_from_boost_serialization = false;
    std::vector<std::string> _image_paths;

    std::string _save_path = DEFAULT_SAVE_PATH;
    ImagePtr _image;
    ImageDescPtr _desc;
    unsigned long _image_idx = 0;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize( Archive & ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_NVP(_image_idx);
        ar & BOOST_SERIALIZATION_NVP(_done_tagging);
        ar & BOOST_SERIALIZATION_NVP(_image_paths);
    }
};
}

#endif //LOCALIZER_CAFFE_MANUELLCLASSIFIER_H