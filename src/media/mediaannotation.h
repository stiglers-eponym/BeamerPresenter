// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef MEDIAANNOTATION_H
#define MEDIAANNOTATION_H

#include <QBuffer>
#include <QByteArray>
#include <QRectF>
#include <QUrl>
#include <memory>

#include "src/config.h"
#include "src/log.h"

/**
 * @brief MediaAnnotation: PDF annotation containing media
 *
 * This class represents media included/embedded/linked in a PDF
 * document, which should be played in a presentation.
 * This is an abstract class.
 *
 * @see EmbeddedMedia, ExternalMedia, EmbeddedAudio
 */
class MediaAnnotation
{
 public:
  /// Subclass type for abstract class MediaAnnotation
  enum Type {
    ExternalURL,          ///< subclass ExternalMedia
    EmbeddedFile,         ///< subclass EmbeddedMedia
    EmbeddedAudioStream,  ///< subclass EmbeddedAudio
  };

  /// Properties of MediaAnnotations
  enum MediaFlag {
    InvalidFlag = 0,
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
  Q_DECLARE_FLAGS(MediaFlags, MediaFlag);
  Q_FLAGS(MediaFlags);

  /// Play modes of media
  enum Mode {
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
  MediaFlags _flags = {};

 public:
  /// Constructor: initialize given parameters
  MediaAnnotation(const QRectF &rect, const Mode mode, const MediaFlags flags)
      : _rect(rect), _mode(mode), _flags(flags)
  {
    debug_verbose(DebugMedia, "creating media annotation" << this);
  }

  /// Trivial destructor
  virtual ~MediaAnnotation()
  {
    debug_verbose(DebugMedia, "deleting media annotation" << this);
  }

  /// Set audio volume (value between 0 and 1)
  void setVolume(const float vol) noexcept { _volume = vol; }

  /// Audio volume (value between 0 and 1)
  float volume() const noexcept { return _volume; }

  /// Set play mode
  void setMode(const Mode mode) noexcept { _mode = mode; }

  /// Play mode
  Mode mode() const noexcept { return _mode; }

  /// Subclass type
  virtual Type type() const noexcept = 0;

  /// Bounding rect of this annotation (in point)
  const QRectF &rect() const noexcept { return _rect; }

  /// Flags (properties) of this object
  MediaFlags flags() const noexcept { return _flags; }

  /// Compare content to other media annotation
  virtual bool operator==(const MediaAnnotation &other) const noexcept = 0;
};

/**
 * @brief ExternalMedia: MediaAnnotation for external or remote files
 *
 * This class covers all media annotations that do not refer to data
 * embedded in the PDF document. This can be a local file, a remote
 * file, a live stream, or a webcam.
 */
class ExternalMedia : public MediaAnnotation
{
  /**
   * Source of the media object. Depending on the URL scheme, the
   * media type is inferred (local file, remote file, live stream,
   * webcam)
   */
  QUrl _url;

 public:
  /// Constructor: Detects type from given URL and adjusts flags based on URL
  /// query
  ExternalMedia(const QUrl &url, const QRectF &rect, const Mode mode,
                const MediaFlags flags = {Interactive | ShowSlider | Autoplay |
                                          HasAudio | HasVideo});

  /// Trivial destructor
  virtual ~ExternalMedia() {}

  virtual Type type() const noexcept override { return ExternalURL; }

  /// Media source
  const QUrl &url() const noexcept { return _url; }

  virtual bool operator==(const MediaAnnotation &other) const noexcept override;
};

/**
 * @brief EmbeddedMedia: MediaAnnotation for a file embedded in the PDF document
 *
 * This class represents a media annotation where the media data is a
 * stream embedded in the PDF document. It is assumed that this
 * stream contains all information (such as file type) requried to be
 * played by QMediaPlayer. For the special case of raw audio data
 * with sampling rate etc. defined in the PDF, the separate class
 * EmbeddedAudio exists.
 */
class EmbeddedMedia : public MediaAnnotation
{
  std::shared_ptr<QByteArray> _data;

 public:
  /// Constructor: initialize given values
  EmbeddedMedia(std::shared_ptr<QByteArray> &data, const QRectF &rect,
                const Mode mode,
                const MediaFlags flags = {Interactive | ShowSlider | Autoplay |
                                          HasAudio | HasVideo})
      : MediaAnnotation(rect, mode, flags), _data(data)
  {
  }

  /// Trivial destructor
  virtual ~EmbeddedMedia() {}

  virtual Type type() const noexcept override { return EmbeddedFile; }

  /// Data representing media.
  std::shared_ptr<QByteArray> &data() noexcept { return _data; }

  virtual bool operator==(const MediaAnnotation &other) const noexcept override;
};

/**
 * @brief EmbeddedAudio: bit stream representing audio with coding defined in
 * PDF
 *
 * This class represents audio data stored in the PDF as raw data
 * stream, with manually specified encoding information such as
 * sampling rate.
 *
 * This class is not functioning yet! I currently don't have an
 * example PDF file for testing this class.
 */
class EmbeddedAudio : public MediaAnnotation
{
  /// Data stream.
  std::shared_ptr<QByteArray> _data;

 public:  // TODO: tidy up later
  /// Audio sampling rate
  int sampling_rate;
  /// Audio channels
  int channels = 1;
  /// Bits per sample
  int bits_per_sample = 8;

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
  }  /// Audio encoding
  encoding = SoundEncodingRaw;

  /// Stream compression modes
  enum Compression {
    /// no compression
    Uncompressed,
  }  /// Stream compression
  compression = Uncompressed;

 public:
  virtual Type type() const noexcept override { return EmbeddedAudioStream; }

  /// Constructor: initialize given values
  EmbeddedAudio(std::shared_ptr<QByteArray> &data, int sampling_rate,
                const QRectF &rect, const Mode mode = Once)
      : MediaAnnotation(rect, mode,
                        {Interactive | ShowSlider | Autoplay | HasAudio}),
        _data(data),
        sampling_rate(sampling_rate)
  {
  }

  /// Trivial destructor
  virtual ~EmbeddedAudio() {}

  /// Comparison by all properties, including beginning of data.
  virtual bool operator==(const MediaAnnotation &other) const noexcept override;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MediaAnnotation::MediaFlags);

#endif  // MEDIAANNOTATION_H
