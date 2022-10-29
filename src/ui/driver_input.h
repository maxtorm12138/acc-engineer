#ifndef ACC_ENGINEER_UI_DRIVER_INPUT_H
#define ACC_ENGINEER_UI_DRIVER_INPUT_H

// qt
#include <QWidget>

// boost
#include <boost/circular_buffer.hpp>

// module
#include "common/defines.h"
#include "shared_memory/defines.h"
#include "shared_memory/shared_memory_emitter.h"

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
    void init_gas_and_throttle_plot();
    void init_speed_plot();
    void init_steering_plot();

    static QString format_gear(int gear);

private:
    Ui::DriverInput *ui_;

    QwtPlotCurve *gas_curve_{nullptr};
    QwtPlotCurve *throttle_curve_{nullptr};
    QwtPlotDirectPainter *gas_and_throttle_painter_{nullptr};

    QwtPlotCurve *speed_curve_{nullptr};
    QwtPlotDirectPainter *speed_painter_{nullptr};

    QwtPlotCurve *steering_curve_{nullptr};
    QwtPlotDirectPainter *steering_painter_{nullptr};

    uint64_t frame_count_{driver_input_dots};

    struct sample
    {
        boost::circular_buffer_space_optimized<float> x_buffer{driver_input_dots, 0};
        boost::circular_buffer_space_optimized<float> y_buffer{driver_input_dots, 0};

        std::array<float, driver_input_dots> x_view{};
        std::array<float, driver_input_dots> y_view{};

        void buffer_to_view();
    };

    sample gas_sample_;
    sample throttle_sample_;
    sample speed_sample_;
};

} // namespace acc_engineer::ui

#endif
