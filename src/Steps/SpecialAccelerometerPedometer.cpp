#include "SpecialAccelerometerPedometer.h"

SpecialAccelerometerPedometer::SpecialAccelerometerPedometer(QObject *parent) {
  Q_UNUSED(parent);

  // initialize values
  m_rollValue = 0.00;
  m_pitchValue = 0.00;
  m_yawValue = 0.00;

  current_sample_number = 1;
  max_accel_sample_number = 0;

  m_forward_accelerations.resize(m_acceleration_vector_size);
  max_accel = 0;
}

/*
 **************************************************
 ************ Roll,Pitch,Yaw Movement *************
 **************************************************
 */

// Roll

// function to return m_rollValue
qreal SpecialAccelerometerPedometer::roll() const { return m_rollValue; }

// function to set m_rollValue
void SpecialAccelerometerPedometer::setRollValue(qreal value) {
  m_rollValue = value;
}

// function to calculate roll value and set it
qreal SpecialAccelerometerPedometer::calculateRoll(qreal x, qreal y, qreal z) {
  return -(qAtan(x / qSqrt(y * y + z * z)) * 57.2957795);
}

// function to set roll with calculate roll
void SpecialAccelerometerPedometer::setRollBasedOnRollCalculatedFromReadings() {
  qreal thisRollValue = SpecialAccelerometerPedometer::calculateRoll(
      SpecialAccelerometerPedometer::reading()->x(),
      SpecialAccelerometerPedometer::reading()->y(),
      SpecialAccelerometerPedometer::reading()->z());
  // qDebug() << "Roll value calculated: " << thisRollValue << "\n";
  // if m_roll_value is not equal to this roll value
  if (SpecialAccelerometerPedometer::roll() != thisRollValue) {
    SpecialAccelerometerPedometer::setRollValue(
        thisRollValue);       // set m_roll_value to thisRollValue
    emit rollValueChanged();  // emit signal for roll value changed
  }
  // qDebug() << "Roll value set: " << SpecialAccelerometerPedometer::roll() <<
  // "\n";
}

// Pitch

// function to return m_pitchValue
qreal SpecialAccelerometerPedometer::pitch() const { return m_pitchValue; }

// function to set m_pitchValue
void SpecialAccelerometerPedometer::setPitchValue(qreal value) {
  m_pitchValue = value;
}

// function to calculate pitch value and return it
qreal SpecialAccelerometerPedometer::calculatePitch(qreal x, qreal y, qreal z) {
  return -(qAtan(y / qSqrt(x * x + z * z)) * 57.2957795);
}

// function to set roll with calculate pitch
void SpecialAccelerometerPedometer::
    setPitchBasedOnPitchCalculatedFromReadings() {
  qreal thisPitchValue = SpecialAccelerometerPedometer::calculatePitch(
      SpecialAccelerometerPedometer::reading()->x(),
      SpecialAccelerometerPedometer::reading()->y(),
      SpecialAccelerometerPedometer::reading()->z());
  // qDebug() << "Pitch value calculated: " << thisRollValue << "\n";
  // if m_pitchValue is not equal to this pitch value
  if (SpecialAccelerometerPedometer::pitch() != thisPitchValue) {
    SpecialAccelerometerPedometer::setPitchValue(
        thisPitchValue);       // set m_pitchValue to thisPitchValue
    emit pitchValueChanged();  // emit signal for roll value changed
  }
  // qDebug() << "Pitch value set: " << SpecialAccelerometerPedometer::pitch()
  // << "\n";
}

// Yaw

// function to return m_yawValue
qreal SpecialAccelerometerPedometer::yaw() const { return m_yawValue; }

// function to set m_yawValue
void SpecialAccelerometerPedometer::setYawValue(qreal value) {
  m_yawValue = value;
}

// function to calculate yaw value and set it
qreal SpecialAccelerometerPedometer::calculateYaw(qreal x, qreal y, qreal z) {
  return -(qAtan(z / qSqrt(y * y + x * x)) * 57.2957795);
}

// function to set roll with calculate roll
void SpecialAccelerometerPedometer::setYawBasedOnYawCalculatedFromReadings() {
  qreal thisYawValue = SpecialAccelerometerPedometer::calculateYaw(
      SpecialAccelerometerPedometer::reading()->x(),
      SpecialAccelerometerPedometer::reading()->y(),
      SpecialAccelerometerPedometer::reading()->z());
  // qDebug() << "Yaw value calculated: " << thisYawValue << "\n";
  // if m_pitchValue is not equal to this pitch value
  if (SpecialAccelerometerPedometer::yaw() != thisYawValue) {
    SpecialAccelerometerPedometer::setYawValue(
        thisYawValue);       // set m_yawValue to thisYawValue
    emit yawValueChanged();  // emit signal for roll value changed
  }
  // qDebug() << "Yaw value set: " << SpecialAccelerometerPedometer::yaw() <<
  // "\n";
}

/*
 ****************************************
 ************ Step Counting *************
 ****************************************
 */
// function to return number of steps
qint8 SpecialAccelerometerPedometer::stepCount() const { return m_num_steps; }
// function to reset number of steps
void SpecialAccelerometerPedometer::resetNumberOfSteps() { m_num_steps = 0; }
// function to set m_num_steps
void SpecialAccelerometerPedometer::setNumberOfSteps(qint8 num) {
  m_num_steps = num;
}
// function to increment m_num_steps
void SpecialAccelerometerPedometer::incrementNumberOfSteps() {
  m_num_steps += 1;
}

// function to run step counting algorithm
void SpecialAccelerometerPedometer::runStepCountAlgorithm() {
  // if sample is valid
  if (SpecialAccelerometerPedometer::isThisSampleValid()) {
    // get current sample number
    qint16 cSampleNum = SpecialAccelerometerPedometer::getCurrentSampleNumber();

    // qDebug() << "In start of Step Count Algorithm. Sample is valid! At " <<
    // cSampleNum << "\n";

    // get max accel sample number
    qint16 maxSampleNum =
        SpecialAccelerometerPedometer::getMaxAccelSampleNumber();

    // the calculated difference between positive peak and negative peak
    // acceleration of a single step session a step is counted if difference
    // between max acceleration value and current acceleration is equal to or
    // more than this
    qreal threshold_difference;

    // calculate threshold difference
    // th = a*(1/(i-k)) + B
    // th = threshold difference acceleration
    //(1/(i-k)) = frequency of steps, i = current sample number, k = max accel
    // sample number a = constant from slope of tangent line of th vs (1/(i-k))
    // B = constant from intercept of tangent line of th vs (1/(i-k))
    threshold_difference =
        (tangent_line_slope_th_freq / (cSampleNum - maxSampleNum)) +
        tangent_line_y_intercept_th_freq;

    // qDebug() << "th: " << threshold_difference << "\n";

    // get current acceleration
    qreal currentAccel = m_forward_accelerations.at(cSampleNum);

    // get maximum acceleration
    qreal maxAccel = SpecialAccelerometerPedometer::getMaxAcceleration();

    // if difference between maximum acceleration and current acceleration is
    // more than
    //  or equal to threshold difference
    if (maxAccel - currentAccel >= threshold_difference) {
      // qDebug() << "Step detected! Current Sample Number: " << cSampleNum <<
      // "\n"; increment number of steps
      SpecialAccelerometerPedometer::incrementNumberOfSteps();
      // emit signal for onStepCountChanged in QML side
      emit stepCountChanged();
      // assign current sample number to max sample number
      SpecialAccelerometerPedometer::setMaxAccelSampleNumber(cSampleNum);
      // assign current acceleration to max acceleration
      SpecialAccelerometerPedometer::setMaxAcceleration(currentAccel);
      // reset current sample number
      SpecialAccelerometerPedometer::resetCurrentSampleNumber();
      // reset max acceleration sample number
      SpecialAccelerometerPedometer::resetMaxAccelSampleNumber();
    }
    // else if difference between maxmimum acceleration and current acceleration
    // is less than threshold difference
    else {
      // if current acceleration is more than max acceleration,
      // assign current acceleration to max acceleration
      if (currentAccel > maxAccel) {
        SpecialAccelerometerPedometer::setMaxAcceleration(currentAccel);
      }
    }
    // increment current sample number
    SpecialAccelerometerPedometer::incrementCurrentSampleNumber();

    // qDebug() << "At end of Step Count Algorithm. Sample is " << cSampleNum <<
    // "\n";
  }
}

// bool for use with step counting algorithm. Tells if next sample is valid for
// use in algorithm.
bool SpecialAccelerometerPedometer::isThisSampleValid() {
  // qDebug() << "Checking for valid sample... \n";

  // get the current forward acceleration accelerometer
  qreal thisForwardAccel = SpecialAccelerometerPedometer::reading()->z();

  // get current sample
  qint16 cSample = SpecialAccelerometerPedometer::getCurrentSampleNumber();

  // if current sample is more than zero
  // and less than 200
  if (cSample > 0 && cSample < m_acceleration_vector_size) {
    // if thisForwardAccel is not equal to previous acceleration
    if (thisForwardAccel != m_forward_accelerations.at(cSample - 1)) {
      // add later, if acceleration is "different" enough from previous
      // acceleration

      // qDebug() << "Valid sample " << cSample << "\n";
      // add thisForwardAccel to m_forward_accelerations vector
      m_forward_accelerations[cSample] = thisForwardAccel;
    }
    // else if it is, return false
    else {
      return false;
    }
  }
  // else if current sample is zero
  else if (cSample == 0) {
    // add thisForwardAccel to m_forward_accelerations vector
    m_forward_accelerations[0] = thisForwardAccel;
  }
  // else if current sample is at limit 200 or more
  else if (cSample >= m_acceleration_vector_size) {
    SpecialAccelerometerPedometer::resetCurrentSampleNumber();  // reset current
                                                                // sample number
    SpecialAccelerometerPedometer::
        resetMaxAccelSampleNumber();  // reset max accel sample number
    m_forward_accelerations[0] =
        thisForwardAccel;  // assign thisForwardAccel to index zero of vector
  }

  return true;
}

// set max_accel
void SpecialAccelerometerPedometer::setMaxAcceleration(qreal num) {
  max_accel = num;
}
// return max_accel
qreal SpecialAccelerometerPedometer::getMaxAcceleration() { return max_accel; }

// increment current_sample_number by 1
void SpecialAccelerometerPedometer::incrementCurrentSampleNumber() {
  current_sample_number += 1;
}
// initialize current_sample_number to one
void SpecialAccelerometerPedometer::resetCurrentSampleNumber() {
  current_sample_number = 1;
}
// return current_sample_number
qint16 SpecialAccelerometerPedometer::getCurrentSampleNumber() {
  return current_sample_number;
}
// set max_accel_sample_number
void SpecialAccelerometerPedometer::setMaxAccelSampleNumber(qint16 thisNum) {
  max_accel_sample_number = thisNum;
}
// return max_accel_sample_number
qint16 SpecialAccelerometerPedometer::getMaxAccelSampleNumber() {
  return max_accel_sample_number;
}
// reset max_accel_sample_number
void SpecialAccelerometerPedometer::resetMaxAccelSampleNumber() {
  max_accel_sample_number = 0;
}

void SpecialAccelerometerPedometer::setTangentLineSlope(qreal num) {
  tangent_line_slope_th_freq = num;
}
qreal SpecialAccelerometerPedometer::tangentLineSlope() const {
  return tangent_line_slope_th_freq;
}

void SpecialAccelerometerPedometer::setTangentLineIntercept(qreal num) {
  tangent_line_y_intercept_th_freq = num;
}
qreal SpecialAccelerometerPedometer::tangentLineIntercept() const {
  return tangent_line_y_intercept_th_freq;
}
