#ifndef ACC_ENGINEER_UI_DRIVER_INPUT_H
#define ACC_ENGINEER_UI_DRIVER_INPUT_H

// qt
#include <QWidget>

// boost
#include <boost/circular_buffer.hpp>

// module
#include "common/defines.h"
#include "shared_memory/defines.h"

// ui forward declaration
namespace Ui {
class DriverInput;
}

// Qwt forward declaration
class QwtPlotCurve;
class QwtPlotDirectPainter;

namespace acc_engineer::ui {

class driver_input final : public QWidget
{
    Q_OBJECT
public:
    explicit driver_input(QWidget *parent = nullptr);

    ~driver_input() override;

public slots:
    void handle_new_frame(shared_memory::frame frame);

private:
    void init_throttle_plot();
    void init_brake_plot();
    void init_speed_plot();
    void init_steering_plot();

    static QString format_gear(int gear);

private:
    Ui::DriverInput *ui_;

    struct sample
    {
        boost::circular_buffer<float> x_buffer{driver_input_dots};
        boost::circular_buffer<float> y_buffer{driver_input_dots};

        std::array<float, driver_input_dots> x_view{};
        std::array<float, driver_input_dots> y_view{};

        void buffer_to_view();
    };

    QwtPlotCurve *throttle_curve_{nullptr};
    sample throttle_sample_{};

    QwtPlotCurve *brake_curve_{nullptr};
    sample brake_sample_{};

    QwtPlotCurve *speed_curve_{nullptr};
    sample speed_sample_{};

    QwtPlotCurve *steering_curve_{nullptr};
    sample steering_sample_{};


};

} // namespace acc_engineer::ui

#endif
