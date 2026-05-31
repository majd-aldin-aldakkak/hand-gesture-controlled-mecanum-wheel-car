# سيارة بعجلات ماكانوم تُتحكّم بإيماءات اليد

> التحكّم بالروبوتات يحتاج عادةً جهاز تحكّم فيزيائي؛ أردت تحكّماً طبيعياً بلا يد مشغولة بأزرار

## نظرة عامة
سيارة بأربع عجلات ماكانوم (حركة 360°)، يتحكّم بها ESP32 يقرأ ميل اليد عبر مستشعر MPU-6050، ويرسل الأوامر لاسلكياً عبر ESP-NOW لـ ESP32 ثانٍ على السيارة.

## المكوّنات والأدوات
- ESP32 × 2
- MPU-6050 IMU
- Mecanum wheels × 4
- DC motors
- motor driver

## كيف يعمل
1. متحكّم ESP32 الأول، المثبّت في كفّ اليد والموصول بمستشعر MPU-6050، يستقبل زوايا الميلان التي ينتجها المستشعر.

2. يعالج ESP32 الأول هذه الزوايا وينتج منها إحداثيات (x, y, z) تمثّل اتجاه حركة اليد.

3. يرسل الإحداثيات على شكل باكيت (Packet) عبر بروتوكول ESP-NOW إلى متحكّم ESP32 الثاني الموجود في السيارة.

4. يحلّل ESP32 الثاني الباكيت المستلَم، ومن خلال معادلات رياضية خاصة بعجلات الماكانوم، يحسب السرعة والاتجاه المطلوبين لكل محرّك على حدة.

5. تدور المحرّكات الأربعة بالسرعات والاتجاهات المحسوبة، فتنتج الحركة المطلوبة — أماماً، خلفاً، جانبياً، أو دوراناً.

## عرض توضيحي
[![Demo](https://img.youtube.com/vi/WQe0gyn4_Zc/0.jpg)](https://youtube.com/watch?v=WQe0gyn4_Zc&si=RpsuR53CaGwMwSU9)

## الكود
الملفات في مجلد `src/`:
- [`hand.ino.ino`](src/hand.ino.ino)
- [`car.ino.ino`](src/car.ino.ino)

## ماذا تعلّمت
vتعلّمت من هذا المشروع استخدام بروتوكول ESP-NOW الذي يربط شريحتَي ESP32 مباشرة دون وسيط، ما أعطى سرعة إرسال واستقبال عالية واستجابة شبه فورية، بمدى يصل إلى ~100 متر.
في وحدة اليد، استخدمت مستشعر MPU-6050 وحسبت زاويتَي الميل (Pitch وRoll). في البداية حسبتهما من مقياس التسارع فقط باستخدام دالة atan2 (التي تعتمد على توزّع الجاذبية على المحاور)، ثم طبّقت مرشّحاً تكاملياً (Complementary Filter) يدمج قراءة الجيروسكوب مع مقياس التسارع: يكامل سرعة الجيروسكوب للنعومة أثناء الحركة، ويستخدم مقياس التسارع لتصحيح الانحراف، فتنتج زاوية أكثر ثباتاً ودقّة. ثم حوّلت الزاويتين إلى أوامر سرعة (vx, vy) عبر دالة map، وأرسلتها كباكيت عبر ESP-NOW.
في السيارة، يستقبل ESP32 الباكيت ويحوّل الأوامر إلى سرعة كل محرّك من محرّكات عجلات الماكانوم الأربعة عبر معادلاتها، مع تطبيع القيم وتحكّم PWM في السرعة.
فهمت أيضاً أن الجيروسكوب يقيس سرعة اللفّ عبر تأثير كوريوليس، وأن مقياس التسارع يقيس الميل عبر الجاذبية، وأن دمجهما بالمرشّح يعطي أفضل ما في الاثنين.

---

# Hand-Gesture-Controlled Mecanum Wheel Car

> Controlling robots usually requires a physical controller; I wanted natural, controller-free motion.

## Overview
A 4-mecanum-wheel car (360° holonomic motion) controlled by an ESP32 reading hand tilt via an MPU-6050 IMU, transmitting commands wirelessly over ESP-NOW to a second ESP32 on the car.

## Components & Tools
- ESP32 × 2
- MPU-6050 IMU
- Mecanum wheels × 4
- DC motors
- motor driver

## How It Works
1. The first ESP32, mounted on the hand and connected to an MPU-6050 sensor, reads the tilt angles produced by the sensor.

2. It processes these angles and generates (x, y, z) coordinates representing the hand's intended direction of motion.

3. It sends these coordinates as a packet over the ESP-NOW protocol to the second ESP32 located on the car.

4. The second ESP32 parses the received packet and, using mecanum-wheel kinematics equations, computes the required speed and direction for each individual motor.

5. The four motors rotate at the computed speeds and directions, producing the desired motion — forward, backward, sideways, or rotation.

## Demo
[![Demo](https://img.youtube.com/vi/WQe0gyn4_Zc/0.jpg)](https://youtube.com/watch?v=WQe0gyn4_Zc&si=RpsuR53CaGwMwSU9)

## Code
Files are in the `src/` folder:
- [`hand.ino.ino`](src/hand.ino.ino)
- [`car.ino.ino`](src/car.ino.ino)

## What I Learned
In this project, I learned to use the ESP-NOW protocol, which connects the two ESP32 boards directly without any intermediary, providing high transmission and reception speed and near-instant response, with a range of up to ~100 meters.
On the hand unit, I used an MPU-6050 sensor and computed the two tilt angles (Pitch and Roll). At first I calculated them from the accelerometer alone using the atan2 function (which relies on how gravity is distributed across the axes). Then I implemented a Complementary Filter that fuses the gyroscope reading with the accelerometer: it integrates the gyroscope's angular rate for smoothness during motion, and uses the accelerometer to correct drift, producing a more stable and accurate angle. I then converted the two angles into speed commands (vx, vy) using the map function and sent them as a packet over ESP-NOW.
On the car, the ESP32 receives the packet and converts the commands into a speed for each of the four mecanum wheel motors through their kinematics equations, with value normalization and PWM speed control.
I also understood that the gyroscope measures rotational speed through the Coriolis effect, while the accelerometer measures tilt through gravity, and that fusing them with a filter gives the best of both.

---
*Documented on 2026-05-31 · Part of my Mechatronics portfolio.*
