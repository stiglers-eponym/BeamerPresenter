#ifndef STROKERECOGNIZER_H
#define STROKERECOGNIZER_H

#include "src/drawing/basicgraphicspath.h"

/**
 * @brief StrokeRecognizer: use only temporarily to analyze a path
 *
 * Objects of this class should only briefly be created for a given path and
 * must be deleted before deleting the path.
 *
 * @todo continue implementing this class
 * @todo use stroke recognizer
 */
class StrokeRecognizer
{
    /// Stroke which should be recognized. The stroke recognizer does not
    /// own this stroke. It must not be deleted while the StrokeRecognizer
    /// is in use.
    const AbstractGraphicsPath *stroke;
    double s = 0., sx = 0., sy = 0., sxx = 0., sxy = 0., syy = 0., sxxy = 0., sxyy = 0.;
public:
    StrokeRecognizer(const AbstractGraphicsPath *stroke) : stroke(stroke) {calc();}
    ~StrokeRecognizer() {}
    void calc() noexcept;
    //BasicGraphicsPath *recognize() const;
    BasicGraphicsPath *recognizeLine() const;
    //BasicGraphicsPath *recognizeRect() const;
    //BasicGraphicsPath *recognizeEllipse() const;
};

#endif // STROKERECOGNIZER_H
