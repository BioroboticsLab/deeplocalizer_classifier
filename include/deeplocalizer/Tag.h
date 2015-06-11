
#ifndef DEEP_LOCALIZER_TAG_H
#define DEEP_LOCALIZER_TAG_H

#include <atomic>

#include <QMetaType>
#include <QString>

#include <opencv2/core/core.hpp>

#include "pipeline/datastructure/Tag.h"
#include "pipeline/datastructure/Ellipse.h"
#include "pipeline/datastructure/serialization.hpp"
#include "serialization.h"

class QPainter;

namespace deeplocalizer {
namespace  tagger {

const int TAG_WIDTH = 96;
const int TAG_HEIGHT = 96;

enum IsTag{
    Yes,
    No,
    Exclude
};

class Tag {
public:
    Tag();
    Tag(const pipeline::Tag & pipetag);
    Tag(cv::Rect boundingBox, boost::optional<pipeline::Ellipse> ellipse);
    static const int IS_TAG_THRESHOLD = 1200;

    unsigned long id() const;
    void setId(unsigned long id);
    const cv::Rect & getBoundingBox() const;
    void setBoundingBox(cv::Rect boundingBox);

    const boost::optional<pipeline::Ellipse> & getEllipse () const;

    IsTag isTag() const;
    void setIsTag(IsTag is_tag);
    void toggleIsTag();

    cv::Mat getSubimage(const cv::Mat &orginal, unsigned int border=0) const;
    bool operator==(const Tag &other) const;
    void guessIsTag(int threshold = IS_TAG_THRESHOLD);
    void draw(QPainter & p, int lineWidth = 3, bool drawVote = true, bool drawEllipse = true);

private:
    unsigned long _id;
    cv::Rect _boundingBox;
    boost::optional<pipeline::Ellipse> _ellipse;
    IsTag _is_tag = IsTag::Yes;

    static unsigned long generateId();
    static std::atomic_long id_counter;
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & BOOST_SERIALIZATION_NVP(_boundingBox);
        ar & BOOST_SERIALIZATION_NVP(_ellipse);
        ar & BOOST_SERIALIZATION_NVP(_is_tag);
    }
};
}
}

Q_DECLARE_METATYPE(deeplocalizer::tagger::Tag)

#endif //DEEP_LOCALIZER_TAG_H
