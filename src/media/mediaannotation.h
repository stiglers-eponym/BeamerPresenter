// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef MEDIAANNOTATION_H
#define MEDIAANNOTATION_H

#include <algorithm>
#include "src/config.h"
#include <QRectF>
#include <QUrl>
#include <QByteArray>
#include <QBuffer>
#include "src/media/mediaitem.h"


class MediaAnnotation
{
public:
    enum Type
    {
        ExternalURL,
        EmbeddedFile,
        EmbeddedAudioStream,
    };

    enum Flags
    {
        /// flag for media annnotation that has audio
        HasAudio = 1 << 0,
        /// flag for media annnotation that has video
        HasVideo = 1 << 1,
        /// should the video start automatically
        Autoplay = 1 << 2,
        /// is this showing a live stream
        IsLive = 1 << 3,
        /// is this should show a media capture session
        IsCaptureSession = 1 << 4,
        /// should a slider be shown
        ShowSlider = 1 << 5,
        /// allow user interaction
        Interactive = 1 << 6,
        /// mute audio
        Mute = 1 << 7,
    };

    /// Play modes of media
    enum Mode
    {
        /// Invlid mode
        InvalidMode = -1,
        /// Play media only once
        Once = 0,
        /// Play video and show controll bar. Currently ignored.
        Open,
        /// Play continuously forward and backward. Currently not implemented.
        Palindrome,
        /// Play repeatedly (infinite loop).
        Repeat,
    };

protected:
    /// audio volume of media
    float _volume = 1.;
    /// position of media on slide
    QRectF _rect;
    /// playing mode
    Mode _mode = Once;
    /// flags
    int _flags = 0;

public:
    MediaAnnotation(const QRectF &rect, Mode mode, const int flags) :
        _rect(rect), _mode(mode), _flags(flags) {}

    virtual ~MediaAnnotation() {}

    void setVolume(const float vol) noexcept
    {_volume = vol;}

    float volume() const noexcept
    {return _volume;}

    void setMode(const Mode mode) noexcept
    {_mode = mode;}

    virtual Type type() const noexcept = 0;

    Mode mode() const noexcept
    {return _mode;}

    const QRectF &rect() const noexcept
    {return _rect;}

    int flags() {return _flags;}

    virtual bool operator==(const MediaAnnotation &other) const noexcept = 0;
};



class ExternalMedia : public MediaAnnotation
{
    QUrl _url;

public:
    ExternalMedia(const QUrl &url, const QRectF &rect, Mode mode);

    virtual Type type() const noexcept override
    {return ExternalURL;}

    const QUrl &url() const noexcept
    {return _url;}

    virtual bool operator==(const MediaAnnotation &other) const noexcept override
    {
        if (type() != other.type() || mode() != other.mode() || rect().toAlignedRect() != other.rect().toAlignedRect())
            return false;
        const auto &other_ext = static_cast<const ExternalMedia&>(other);
        return _url == other_ext._url;
    }
};



class EmbeddedMedia : public MediaAnnotation
{
    QByteArray _data;
    QBuffer _buffer;

public:
    EmbeddedMedia(QByteArray &data, const QRectF &rect, Mode mode) :
        MediaAnnotation(rect, mode, Interactive|ShowSlider|Autoplay|HasAudio|HasVideo), _data(data), _buffer(&_data)
        {_buffer.open(QBuffer::ReadOnly);}

    virtual Type type() const noexcept override
    {return EmbeddedFile;}

    QBuffer &buffer() noexcept
    {return _buffer;}

    virtual bool operator==(const MediaAnnotation &other) const noexcept override
    {
        if (type() != other.type() || mode() != other.mode() || rect().toAlignedRect() != other.rect().toAlignedRect())
            return false;
        const auto &other_em = static_cast<const EmbeddedMedia&>(other);
#if (QT_VERSION_MAJOR >= 6)
        return _data.size() == other_em._data.size()
               && other_em._data.startsWith(_data.first(std::min(_data.size(), qsizetype(32))));
#else
        return _data.size() == other_em._data.size();
#endif
    }
};



/// Embedded audio stream with properties defined in PDF.
class EmbeddedAudio : public MediaAnnotation
{
    /// Data stream.
    QByteArray data;
public: // TODO: tidy up later
    /// Audio sampling rate
    int sampling_rate;
    /// Audio channels
    int channels = 1;
    /// Bit per sample
    int bit_per_sample = 8;

    /// Audio encoding modes as defined by PDF standard
    enum Encoding {
        /// Raw unsigned integers between 0 and 2^8-1
        SoundEncodingRaw,
        /// Twos-complement values
        SoundEncodingSigned,
        /// mu-law encoded samples
        SoundEncodingMuLaw,
        /// A-law-encoded samples
        SoundEncodingALaw,
    }
    /// Audio encoding
    encoding = SoundEncodingRaw;

    /// Stream compression modes
    enum Compression {
        /// no compression
        Uncompressed,
    }
    /// Stream compression
    compression = Uncompressed;

public:
    virtual Type type() const noexcept override {return EmbeddedAudioStream;}

    /// Constructor
    EmbeddedAudio(const QByteArray &data, int sampling_rate, const QRectF &rect, const Mode mode=Once) :
        MediaAnnotation(rect, mode, Interactive|ShowSlider|Autoplay|HasAudio),
        data(data),
        sampling_rate(sampling_rate)
    {}

    /// Trivial destructor
    virtual ~EmbeddedAudio() {}

    /// Comparison by all properties, including data.
    virtual bool operator==(const MediaAnnotation &other) const noexcept override
    {
        if (type() != other.type() || mode() != other.mode() || rect().toAlignedRect() != other.rect().toAlignedRect())
            return false;
        const auto &other_em = static_cast<const EmbeddedAudio&>(other);
#if (QT_VERSION_MAJOR >= 6)
        return data.size() == other_em.data.size()
               && encoding == other_em.encoding
               && other_em.data.startsWith(data.first(std::min(data.size(), qsizetype(32))));
#else
        return data.size() == other_em.data.size();
#endif
    }
};

#endif // MEDIAANNOTATION_H
