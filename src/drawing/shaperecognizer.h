// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef SHAPERECOGNIZER_H
#define SHAPERECOGNIZER_H

#include <cmath>
#include <QtGlobal>
#include <QList>
#include "src/config.h"

class BasicGraphicsPath;
class AbstractGraphicsPath;

/**
 * @brief ShapeRecognizer: use only temporarily to analyze a path
 *
 * Objects of this class should only briefly be created for a given path and
 * must be deleted before deleting the path.
 */
class ShapeRecognizer
{
    /// Line fitted to a set of points
    struct Line {
        qreal bx;     ///< x coordinate of a point on the line
        qreal by;     ///< y coordinate of a point on the line
        qreal angle;  ///< between -pi and pi
        qreal weight; ///< variance, positive number
        qreal loss;   ///< value of loss function, positive number
    };

    /// Collection of statistical moments (0th, 1st and 2nd) of a set of points,
    /// together with functions for fitting a line.
    struct Moments {
        qreal s = 0.;   ///< total weight
        qreal sx = 0.;  ///< weighted sum of x coordinates
        qreal sy = 0.;  ///< weighted sum of y coordinates
        qreal sxx = 0.; ///< weighted sum of x*x
        qreal sxy = 0.; ///< weighted sum of x*y
        qreal syy = 0.; ///< weighted sum of y*y
        /// Add moments (all moments are linear)
        Moments operator+(const Moments& other) const noexcept
        {return {s+other.s, sx+other.sx, sy+other.sy, sxx+other.sxx, sxy+other.sxy, syy+other.syy};}
        /// Add moments (all moments are linear)
        void operator+=(const Moments& other) noexcept
        {s+=other.s; sx+=other.sx; sy+=other.sy; sxx+=other.sxx; sxy+=other.sxy; syy+=other.syy;}
        /// Reset all moments and weights to 0.
        void reset() noexcept
        {s=sx=sy=sxx=sxy=syy=0;}
        /// Compute variance (normalized to weights)
        qreal var() const noexcept
        {return (sxx - sx*sx/s + syy - sy*sy/s)/s;}
        /// Compute standard deviation (square root of variance, normalized to weights).
        qreal std() const noexcept
        {return std::sqrt(s*sxx - sx*sx + s*syy - sy*sy)/s;}
        /// Fit a line to the given moments.
        Line line(const bool calc_weight = true) const noexcept
        {
            const qreal
                    n = sy*sy - s*syy + s*sxx - sx*sx,
                    d = 2*(sx*sy - s*sxy),
                    ay = n - std::sqrt(n*n + d*d),
                    loss = (d*d*(s*syy-sy*sy) + ay*ay*(s*sxx-sx*sx) + 2*d*ay*(sx*sy-s*sxy)) / ((d*d+ay*ay) * (s*sxx - sx*sx + s*syy - sy*sy)),
                    weight = calc_weight ? std() : 0.,
                    angle = std::atan2(ay, d);
            return {sx/s, sy/s, angle > M_PI ? angle - M_PI : angle < -M_PI ? angle + M_PI : angle, weight, loss};
        }
    };

    /// Path which should be recognized. The shape recognizer does not
    /// own this path. It must not be deleted while the ShapeRecognizer
    /// is in use.
    const AbstractGraphicsPath *path;

    /// Lines recognized in this path.
    QList<Line> line_segments;

    /// 0th, 1st and 2nd moments
    Moments moments;
    qreal   sxxx = 0., ///< weighted sum of x*x*x
            sxxy = 0., ///< weighted sum of x*x*y
            sxyy = 0., ///< weighted sum of x*y*y
            syyy = 0., ///< weighted sum of y*y*y
            sxxxx = 0., ///< weighted sum of x*x*x*x
            sxxyy = 0., ///< weighted sum of x*x*y*y
            syyyy = 0.; ///< weighted sum of y*y*y*y

    /**
     * Loss function for ellipse fit.
     * @param mx x coordinate of center
     * @param my y coordinate of center
     * @param ax = 1/(rx*rx) = inverse radius in x-direction squared
     * @param ay = 1/(ry*ry) = inverse radius in y-direction squared
     *
     * The parameters ax = 1/(rx*rx) and ay = 1/(ry*ry) are used instead of rx
     * and ry to make this function a polynomial.
     *
     * This returns
     * \[ \sum_k w_k \left[ \frac{(x_k-m_x)^2}{rx^2} + \frac{(y_k-m_y)^2}{ry^2} - 1 \right]^2 \]
     * where (x_k, y_k) is a coordinate with weight w_k.
     */
    qreal ellipseLossFunc(const qreal mx, const qreal my, const qreal ax, const qreal ay) const noexcept
    {
        const qreal bc = mx*mx*ax + my*my*ay - 1;
        return
                moments.s*bc*bc
                + sxxxx*ax*ax
                + syyyy*ay*ay
                + 2*sxxyy*ax*ay
                + 2*(2*mx*mx*ax + bc)*moments.sxx*ax
                + 2*(2*my*my*ay + bc)*moments.syy*ay
                + 8*mx*my*moments.sxy*ax*ay
                - 4*mx*ax*(sxxx*ax + sxyy*ay)
                - 4*my*ay*(sxxy*ax + syyy*ay)
                - 4*bc*(mx*moments.sx*ax + my*moments.sy*ay);
    }
    /**
     * First derivative of ellipseLossFunc by mx.
     * @see ellipseLossFunc
     */
    qreal ellipseLossGradient_mx(const qreal mx, const qreal my, const qreal ax, const qreal ay) const noexcept
    {
        const qreal bc = mx*mx*ax + my*my*ay - 1;
        return
                4*moments.s*bc*mx*ax
                + 12*mx*moments.sxx*ax*ax
                + 4*mx*moments.syy*ax*ay
                + 8*my*moments.sxy*ax*ay
                - 4*ax*(sxxx*ax + sxyy*ay)
                - 4*bc*moments.sx*ax
                - 8*mx*ax*(mx*moments.sx*ax + my*moments.sy*ay);
    }
    /**
     * First derivative of ellipseLossFunc by my.
     * @see ellipseLossFunc
     */
    qreal ellipseLossGradient_my(const qreal mx, const qreal my, const qreal ax, const qreal ay) const noexcept
    {
        const qreal bc = mx*mx*ax + my*my*ay - 1;
        return
                4*moments.s*bc*my*ay
                + 12*my*moments.syy*ay*ay
                + 4*my*moments.sxx*ax*ay
                + 8*mx*moments.sxy*ax*ay
                - 4*ay*(sxxy*ax + syyy*ay)
                - 4*bc*moments.sy*ay
                - 8*my*ay*(mx*moments.sx*ax + my*moments.sy*ay);
    }
    /**
     * First derivative of ellipseLossFunc by ax.
     * @see ellipseLossFunc
     */
    qreal ellipseLossGradient_ax(const qreal mx, const qreal my, const qreal ax, const qreal ay) const noexcept
    {
        const qreal bc = mx*mx*ax + my*my*ay - 1;
        return
                2*moments.s*mx*mx*bc
                + 2*sxxxx*ax
                + 2*sxxyy*ay
                + 2*(5*mx*mx*ax + bc)*moments.sxx
                + 2*mx*mx*moments.syy*ay
                + 8*mx*my*moments.sxy*ay
                - 4*mx*(2*sxxx*ax + sxyy*ay)
                - 4*my*ay*sxxy
                - 4*mx*mx*(mx*moments.sx*ax + my*moments.sy*ay)
                - 4*bc*mx*moments.sx;
    }
    /**
     * First derivative of ellipseLossFunc by ay.
     * @see ellipseLossFunc
     */
    qreal ellipseLossGradient_ay(const qreal mx, const qreal my, const qreal ax, const qreal ay) const noexcept
    {
        const qreal bc = mx*mx*ax + my*my*ay - 1;
        return
                2*moments.s*my*my*bc
                + 2*syyyy*ay
                + 2*sxxyy*ax
                + 2*(5*my*my*ay + bc)*moments.syy
                + 2*my*my*moments.sxx*ax
                + 8*mx*my*moments.sxy*ax
                - 4*my*(sxxy*ax + 2*syyy*ay)
                - 4*mx*ax*sxyy
                - 4*my*my*(mx*moments.sx*ax + my*moments.sy*ay)
                - 4*bc*my*moments.sy;
    }

public:
    /// Trivial constructor.
    ShapeRecognizer(const AbstractGraphicsPath *path) : path(path) {}

    /// Trivial destructor.
    ~ShapeRecognizer() {}

    /// Compute third and fourth statistical moments of points in path up.
    void calc_higher_moments() noexcept;

    /// Recognize line segments in this stoke.
    void findLines() noexcept;

    /// Try to recognize a known shape in path.
    /// Return NULL if no shape was detected.
    BasicGraphicsPath *recognize();

    /// Check if path is a line.
    /// Return a BasicGraphicsPath* representing this line if successful, NULL otherwise.
    BasicGraphicsPath *recognizeLine() const;

    /// Check if path is a rectangle.
    /// Return a BasicGraphicsPath* representing this line if successful, NULL otherwise.
    BasicGraphicsPath *recognizeRect() const;

    /// Check if path is an ellipse.
    /// Return a BasicGraphicsPath* representing this line if successful, NULL otherwise.
    BasicGraphicsPath *recognizeEllipse() const;
};

#endif // SHAPERECOGNIZER_H
