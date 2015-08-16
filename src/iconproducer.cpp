#include "iconproducer.h"
#include <LXQt/Settings>
#include <XdgIcon>
#include <QDebug>
#include <QtSvg/QSvgRenderer>
#include <QPainter>
#include <math.h>

IconProducer::IconProducer(Solid::Battery *battery, QObject *parent) : QObject(parent)
{
    connect(battery, &Solid::Battery::chargeStateChanged, this, &IconProducer::updateState);
    connect(battery, &Solid::Battery::chargePercentChanged, this, &IconProducer::updateChargePercent);
    connect(&mSettings, SIGNAL(settingsChanged()), this, SLOT(update()));

    mChargePercent = battery->chargePercent();
    mState = battery->chargeState();
    themeChanged();
}

IconProducer::IconProducer(QObject *parent):  QObject(parent)
{
    themeChanged();
    update();
}


void IconProducer::updateChargePercent(int newChargePercent)
{
    mChargePercent = newChargePercent;

    update();
}

void IconProducer::updateState(int newState)
{
    mState = (Solid::Battery::ChargeState) newState;

    update();
}

void IconProducer::update()
{
    QString newIconName;

    if (mSettings.isUseThemeIcons())
    {
        QMap<float, QString> *levelNameMap = (mState == Solid::Battery::Discharging ? &mLevelNameMapDischarging : &mLevelNameMapCharging);
        foreach (float level, levelNameMap->keys())
        {
            if (level >= mChargePercent)
            {
                newIconName = levelNameMap->value(level);
                break;
            }
        }
    }

    if (mSettings.isUseThemeIcons() && newIconName == mIconName)
        return;

    mIconName = newIconName;
    mIcon = mSettings.isUseThemeIcons() ? QIcon::fromTheme(mIconName) : circleIcon();
    emit iconChanged();
}

void IconProducer::themeChanged()
{
    /*
     * We maintain specific charge-level-to-icon-name mappings for Oxygen and Awoken and
     * asume all other themes adhere to the freedesktop standard.
     * This is certainly not true, and as bug reports come in stating that
     * this and that theme is not working we will probably have to add new
     * mappings beside Oxygen and Awoken
     */
    mLevelNameMapDischarging.clear();
    mLevelNameMapCharging.clear();

    if (QIcon::themeName() == "oxygen")
    {
        // Means:
        // Use 'battery-low' for levels up to 10
        //  -  'battery-caution' for levels between 10 and 20
        //  -  'battery-040' for levels between 20 and 40, etc..
        mLevelNameMapDischarging[10] = "battery-low";
        mLevelNameMapDischarging[20] = "battery-caution";
        mLevelNameMapDischarging[40] = "battery-040";
        mLevelNameMapDischarging[60] = "battery-060";
        mLevelNameMapDischarging[80] = "battery-080";
        mLevelNameMapDischarging[101] = "battery-100";
        mLevelNameMapCharging[10] = "battery-charging-low";
        mLevelNameMapCharging[20] = "battery-charging-caution";
        mLevelNameMapCharging[40] = "battery-charging-040";
        mLevelNameMapCharging[60] = "battery-charging-060";
        mLevelNameMapCharging[80] = "battery-charging-080";
        mLevelNameMapCharging[101] = "battery-charging";
    }
    else if (QIcon::themeName().startsWith("AwOken")) // AwOken, AwOkenWhite, AwOkenDark
    {
        mLevelNameMapDischarging[5] = "battery-000";
        mLevelNameMapDischarging[30] = "battery-020";
        mLevelNameMapDischarging[50] = "battery-040";
        mLevelNameMapDischarging[70] = "battery-060";
        mLevelNameMapDischarging[95] = "battery-080";
        mLevelNameMapDischarging[101] = "battery-100";
        mLevelNameMapCharging[5] = "battery-000-charging";
        mLevelNameMapCharging[30] = "battery-020-charging";
        mLevelNameMapCharging[50] = "battery-040-charging";
        mLevelNameMapCharging[70] = "battery-060-charging";
        mLevelNameMapCharging[95] = "battery-080-charging";
        mLevelNameMapCharging[101] = "battery-100-charging";
    }
    else // As default we fall back to the freedesktop scheme.
    {
        mLevelNameMapDischarging[3] = "battery-empty";
        mLevelNameMapDischarging[10] = "battery-caution";
        mLevelNameMapDischarging[50] = "battery-low";
        mLevelNameMapDischarging[90] = "battery-good";
        mLevelNameMapDischarging[101] = "battery-full";
        mLevelNameMapCharging[3] = "battery-empty";
        mLevelNameMapCharging[10] = "battery-caution-charging";
        mLevelNameMapCharging[50] = "battery-low-charging";
        mLevelNameMapCharging[90] = "battery-good-charging";
        mLevelNameMapCharging[101] = "battery-full-charging";
    }

    update();
}

QIcon& IconProducer::circleIcon()
{
    static QMap<Solid::Battery::ChargeState, QMap<int, QIcon> > cache;

    int chargeLevelAsInt = (int) (mChargePercent + 0.49);

    if (!cache[mState].contains(chargeLevelAsInt))
        cache[mState][chargeLevelAsInt] = buildCircleIcon(mState, mChargePercent);

    return cache[mState][chargeLevelAsInt];
}

QIcon IconProducer::buildCircleIcon(Solid::Battery::ChargeState state, int chargeLevel)
{
    static QString svg_template =
        "<svg\n"
        "    xmlns:dc='http://purl.org/dc/elements/1.1/'\n"
        "    xmlns:cc='http://creativecommons.org/ns#'\n"
        "    xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#'\n"
        "    xmlns:svg='http://www.w3.org/2000/svg'\n"
        "    xmlns='http://www.w3.org/2000/svg'\n"
        "    version='1.1'\n"
        "    width='200'\n"
        "    height='200'>\n"
        "\n"
        "<defs>\n"
        "    <linearGradient id='greenGradient' x1='0%' y1='0%' x2='100%' y2='100%'>\n"
        "        <stop offset='0%' style='stop-color:rgb(125,255,125);stop-opacity:1' />\n"
        "        <stop offset='150%' style='stop-color:rgb(15,125,15);stop-opacity:1' />\n"
        "    </linearGradient>\n"
        "</defs>\n"
        "\n"
        "<circle cx='100' cy='100' r='99' style='fill:#FFFFFF;stroke:none; opacity:0.2;'/>\n"
        "<path d='M 100,20 A80,80 0, LARGE_ARC_FLAG, SWEEP_FLAG, END_X,END_Y' style='fill:none; stroke:url(#greenGradient); stroke-width:38;' />\n"
        "<path d='M 100,20 A80,80 0, LARGE_ARC_FLAG, SWEEP_FLAG, END_X,END_Y' style='fill:none; stroke:red; stroke-width:38; opacity:RED_OPACITY' />\n"
        "\n"
        " STATE_MARKER\n"
        "\n"
        "</svg>";

    static QString filledCircle   = "<circle cx='100' cy='100' r='35'/>";
    static QString plus           = "<path d='M 60,100 L140,100 M100,60 L100,140' style='stroke:black; stroke-width:30;'/>";
    static QString minus          = "<path d='M 60,100 L140,100' style='stroke:black; stroke-width:30;'/>";
    static QString hollowCircle   = "<circle cx='100' cy='100' r='30' style='fill:none;stroke:black;stroke-width:10'/>";

    QString svg = svg_template;

    if (chargeLevel > 99)
        chargeLevel = 99;

	double angle;
    QString sweepFlag;
	if (state == Solid::Battery::Discharging)
	{
		 angle = M_PI_2 + 2 * M_PI * chargeLevel/100;
         sweepFlag = "0";
	}
	else 
	{
		angle = M_PI_2 - 2 *M_PI * chargeLevel/100;
		sweepFlag = "1";
	}
    double circle_endpoint_x = 80.0 * cos(angle) + 100;
    double circle_endpoint_y = -80.0 * sin(angle) + 100;

    QString largeArgFlag = chargeLevel > 50 ? "1" : "0";

    svg.replace(QString("END_X"), QString::number(circle_endpoint_x));
    svg.replace(QString("END_Y"), QString::number(circle_endpoint_y));
    svg.replace(QString("LARGE_ARC_FLAG"), largeArgFlag);
    svg.replace(QString("SWEEP_FLAG"), sweepFlag);

    switch (state)
    {
    case Solid::Battery::FullyCharged:
        svg.replace("STATE_MARKER", filledCircle);
        break;
    case Solid::Battery::Charging:
        svg.replace("STATE_MARKER", plus);
        break;
    case Solid::Battery::Discharging:
        svg.replace("STATE_MARKER", minus);
        break;
    default:
        svg.replace("STATE_MARKER", hollowCircle);
    }

    if (state != Solid::Battery::FullyCharged && state != Solid::Battery::Charging &&  chargeLevel < mSettings.getPowerLowLevel() + 30)
    {
        if (chargeLevel <= mSettings.getPowerLowLevel() + 10)
            svg.replace("RED_OPACITY", "1");
        else
            svg.replace("RED_OPACITY", QString::number((mSettings.getPowerLowLevel() + 30 - chargeLevel)/20.0));
    }
    else
        svg.replace("RED_OPACITY", "0");

    // qDebug() << svg;

    // Paint the svg on a pixmap and create an icon from that.
    QSvgRenderer render(svg.toLatin1());
    QPixmap pixmap(render.defaultSize());
    pixmap.fill(QColor(0,0,0,0));
    QPainter painter(&pixmap);
    render.render(&painter);
    return QIcon(pixmap);
}
