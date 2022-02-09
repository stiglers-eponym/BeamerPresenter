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
 * @todo implement simpler way of accessing stroke recognizer
 */
class StrokeRecognizer
{
    /// Stroke which should be recognized. The stroke recognizer does not
    /// own this stroke. It must not be deleted while the StrokeRecognizer
    /// is in use.
    const AbstractGraphicsPath *stroke;

    double  s = 0., ///< sum of weights (pressures)
            sx = 0., ///< weighted sum of of x coordinates
            sy = 0., ///< weighted sum of of y coordinates
            sxx = 0., ///< weighted sum of of x*x
            sxy = 0., ///< weighted sum of of x*y
            syy = 0.; ///< weighted sum of of y*y

public:
    /// Constructor. Computes statistical moments of the given path.
    StrokeRecognizer(const AbstractGraphicsPath *stroke) : stroke(stroke)
    {calc();}

    /// Trivial destructor.
    ~StrokeRecognizer() {}

    /// Compute statistical moments of points in stroke.
    void calc() noexcept;

    /// Try to recognize a known shape in stroke.
    /// Return NULL if no shape was detected.
    BasicGraphicsPath *recognize() const;

    /// Check if stroke is a line.
    /// Return a BasicGraphicsPath* representing this line if successful, NULL otherwise.
    BasicGraphicsPath *recognizeLine() const;

    /// Check if stroke is a rectangle.
    /// Return a BasicGraphicsPath* representing this line if successful, NULL otherwise.
    /// @todo not implemented yet
    BasicGraphicsPath *recognizeRect() const;

    /// Check if stroke is an ellipse.
    /// Return a BasicGraphicsPath* representing this line if successful, NULL otherwise.
    /// @todo not implemented yet
    BasicGraphicsPath *recognizeEllipse() const;
};

#endif // STROKERECOGNIZER_H
