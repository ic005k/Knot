#ifndef SPECIALACCELEROMETERPEDOMETER_H
#define SPECIALACCELEROMETERPEDOMETER_H

#include <QAccelerometer>
#include <QDebug>  //for qDebug
#include <QObject>
#include <QVector>  //for QVector
#include <QtMath>   //for qAtan, qSqrt

/*
 * Sample class
class RandomNumberGenerator : public QObject, public QQmlPropertyValueSource
{
    Q_OBJECT
    Q_INTERFACES(QQmlPropertyValueSource)
    Q_PROPERTY(int maxValue READ maxValue WRITE setMaxValue NOTIFY
maxValueChanged);

RandomNumberGenerator(QObject *parent)
        : QObject(parent), m_maxValue(100)
*/

// class inherits publicly from QAccelerometer and publicly for
// QQmlPropertyValueSource
class SpecialAccelerometerPedometer : public QAccelerometer {
  Q_OBJECT

  // value name qml, READ value to read, WRITE function to set value, NOTIFY
  // signal function
  Q_PROPERTY(qreal roll READ roll WRITE setRollValue NOTIFY
                 rollValueChanged)  // roll property
  Q_PROPERTY(qreal pitch READ pitch WRITE setPitchValue NOTIFY
                 pitchValueChanged)  // pitch property
  Q_PROPERTY(qreal yaw READ yaw WRITE setYawValue NOTIFY
                 yawValueChanged)  // yaw property
  Q_PROPERTY(qreal stepCount READ stepCount WRITE setNumberOfSteps NOTIFY
                 stepCountChanged)  // step count property
  Q_PROPERTY(qreal tangent_line_y_intercept READ tangentLineSlope WRITE
                 setTangentLineSlope NOTIFY
                     tangentLineSlopeChanged)  // tangent line slope property
  Q_PROPERTY(
      qreal tangent_line_slope READ tangentLineIntercept WRITE
          setTangentLineIntercept NOTIFY
              tangentLineInterceptChanged)  // tangent line intercept property

 public:
  // constructor
  SpecialAccelerometerPedometer(QObject* parent = nullptr);

  // roll, forward direction
  // function to return m_rollValue
  qreal roll() const;
  // function to set m_rollValue
  void setRollValue(qreal value);
  // function to return calculated roll value from inputs
  qreal calculateRoll(qreal x, qreal y, qreal z);
  // function to set roll with calculate roll
  void setRollBasedOnRollCalculatedFromReadings();
  // function to invoke from QML side, calls C++ function
  // setRollBasedOnRollCalculatedFromReadings()
  Q_INVOKABLE void calcRollFromReadings() {
    SpecialAccelerometerPedometer::setRollBasedOnRollCalculatedFromReadings();
  }

  // pitch, side direction
  // function to return m_pitchValue
  qreal pitch() const;
  // function to set m_pitchValue
  void setPitchValue(qreal value);
  // function to return calculated pitch value from inputs
  qreal calculatePitch(qreal x, qreal y, qreal z);
  // function to set roll with calculate pitch
  void setPitchBasedOnPitchCalculatedFromReadings();
  // function to invoke from QML side, calls C++ function
  // setPitchBasedOnPitchCalculatedFromReadings()
  Q_INVOKABLE void calcPitchFromReadings() {
    SpecialAccelerometerPedometer::setPitchBasedOnPitchCalculatedFromReadings();
  }

  // yaw, vertical direction
  // function to return m_rollValue
  qreal yaw() const;
  // function to set m_rollValue
  void setYawValue(qreal value);
  // function to return calculated roll value from inputs
  qreal calculateYaw(qreal x, qreal y, qreal z);
  // function to set roll with calculate roll
  void setYawBasedOnYawCalculatedFromReadings();
  // function to invoke from QML side, calls C++ function
  // setRollBasedOnRollCalculatedFromReadings()
  Q_INVOKABLE void calcYawFromReadings() {
    SpecialAccelerometerPedometer::setYawBasedOnYawCalculatedFromReadings();
  }

  // Step Counting Algorithm processes
  // function to return number of steps
  qint8 stepCount() const;
  // function to reset number of steps
  void resetNumberOfSteps();
  // function to set m_num_steps
  void setNumberOfSteps(qint8 num);
  // function to increment m_num_steps
  void incrementNumberOfSteps();

  // function to run step counting algorithm
  void runStepCountAlgorithm();

  void setTangentLineSlope(qreal num);
  qreal tangentLineSlope() const;

  void setTangentLineIntercept(qreal num);
  qreal tangentLineIntercept() const;

  // function to invoke from QML side, calls C++ function resetNumberOfSteps()
  Q_INVOKABLE void resetStepCount() {
    SpecialAccelerometerPedometer::resetNumberOfSteps();
  }

  // function to invoke from QML side, calls C++ function
  // runStepCountAlgorithm()
  Q_INVOKABLE void runQMLStepCountAlgorithm() {
    SpecialAccelerometerPedometer::runStepCountAlgorithm();
  }

  // QML Property Definition for SpecialAccelerometerPedometer
  /* When the QML engine encounters a use of SpecialAccelerometerPedometer
   * as a property value source, it invokes
   * SpecialAccelerometerPedometer::setTarget() to provide the type with the
   * property to which the value source has been applied. */

 signals:
  // function to run when m_rollValue is changed
  void rollValueChanged();

  // function to run when m_pitchValue is changed
  void pitchValueChanged();

  // function to run when m_yawValue is changed
  void yawValueChanged();

  // function to run when m_num_steps is changed
  void stepCountChanged();

  // function to run when tangent_line_slope_th_freq is changed
  void tangentLineSlopeChanged();

  // function to run when tangent_line_intercept_th_freq is changed
  void tangentLineInterceptChanged();

 private slots:

 private:
  // Calculated data members from Accelerometer readings

  // Calculated members for movement in a direction

  qreal m_rollValue;   // how much moved in forward direction
  qreal m_pitchValue;  // how much moved in side direction
  qreal m_yawValue;    // how much moved in vertical direction

  // Calculated members for step counting algorithm
  /*
   *************** STEP COUNTING ALGORITHM ***********************
   */

  // number of steps counted,
  // in 8-bit range b/c realistically no one will walk more than 256 steps
  // in one moving session for this application.
  qint8 m_num_steps;
  // vector to hold acceleration values in forward axis direction relative to
  // person and not phone
  QVector<qreal> m_forward_accelerations;
  qint16 const m_acceleration_vector_size = 270;

  qreal max_accel;  // the positive peak maximum acceleration recorded
  void setMaxAcceleration(qreal num);  // set max_accel
  qreal getMaxAcceleration();          // return max_accel

  qreal
      tangent_line_slope_th_freq;  // the slope from tangent line from graphing
                                   // threshold difference vs frequency

  qreal tangent_line_y_intercept_th_freq;  // the y-intercept from tangent line
                                           // from graphing threshold diff vs
                                           // frequency

  qint16 current_sample_number;  // the sample number of data the algorithm is
                                 // currently on
  void incrementCurrentSampleNumber();  // increment current_sample_number by 1
  void resetCurrentSampleNumber();  // initialize current_sample_number to zero
  qint16 getCurrentSampleNumber();  // return current_sample_number

  qint16 max_accel_sample_number;  // the sample number when max acceleration
                                   // was recorded
  void setMaxAccelSampleNumber(qint16 thisNum);  // set max_accel_sample_number
  qint16 getMaxAccelSampleNumber();  // return max_accel_sample_number
  void resetMaxAccelSampleNumber();  // reset max_accel_sample_number

  bool isThisSampleValid();  // bool for use with step counting algorithm. Tells
                             // if next sample is valid for use in algorithm.
};

#endif  // SPECIALACCELEROMETERPEDOMETER_H
