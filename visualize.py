import argparse
from collections import defaultdict
import csv
from datetime import datetime

import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib.dates import MONDAY
from matplotlib.dates import DayLocator, WeekdayLocator, DateFormatter


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('input_file')
    parser.add_argument('output_file')

    args = parser.parse_args()

    data = read_data(args.input_file)

    graph_data(data, args.output_file)


def read_data(filename):
    data = defaultdict(list)

    with open(filename) as fp:
        reader = csv.DictReader(fp)
        for record in reader:
            dt_tuple = (record['year'], record['month'], record['day'],
                        record['hour'], record['minute'], record['second'])
            dt_tuple = map(int, dt_tuple)
            dt = datetime(*dt_tuple)

            data['datetime'].append(dt)
            keys = ('DHT temp (C)',
                    'DHT humidity (%)',
                    'BMP temp (C)',
                    'BMP pressure (hPa)',
                    'DS3231 temp (C)',)

            for key in keys:
                value = float(record.get(key, -2))
                data[key].append(value)

    return data


def graph_data(data, filename):
    fig, axs = plt.subplots(
        len(data) - 1,
        sharex=True,
        figsize=(10, 15),
    )

    mondays = WeekdayLocator(MONDAY)
    days = DayLocator()
    daysFmt = DateFormatter("%Y-%m-%d")

    y_formatter = mpl.ticker.ScalarFormatter(useOffset=False)

    datetimes = data.get('datetime')
    axes_cntr = 0
    for key in sorted(data):
        if key == 'datetime':
            continue

        values = data.get(key)

        ax = axs[axes_cntr]
        axes_cntr += 1

        ax.plot(datetimes, values, label=key)
        ax.set_title(key)
        ax.xaxis.set_major_locator(mondays)
        ax.xaxis.set_major_formatter(daysFmt)
        ax.xaxis.set_minor_locator(days)
        ax.yaxis.set_major_formatter(y_formatter)
        ax.autoscale_view()

    # fig.subplots_adjust(hspace=10)
    fig.autofmt_xdate()
    plt.savefig(filename, dpi=300)


if __name__ == "__main__":
    main()

