import os
import csv
import pandas as pd
from pathlib import Path
import plotly.express as px
import re
import dash_bootstrap_components as dbc
from dash import Dash, dcc, callback, Output, Input


def load_results_dataframe(base_dir: Path) -> pd.DataFrame:
    """
    Reads report.csv files from subdirectories of base_dir
    and builds a pandas DataFrame with collected results.
    """
    full_df = pd.DataFrame()

    for subdir in os.listdir(base_dir):
        subdir_path = os.path.join(base_dir, subdir)
        result_file = os.path.join(subdir_path, "report.csv")

        if not os.path.isdir(subdir_path):
            continue
        if not os.path.exists(result_file):
            continue

        match = re.search(r"hpc-performance_web-(\w+)_(\d+)", subdir)
        if not match:
            continue  # skip if pattern doesn't match

        page_name = match.group(1)
        threads = int(match.group(2))

        fields = {"Page Name": page_name, "Threads": threads}

        with open(result_file, newline="", encoding="utf-8") as f:
            reader = csv.reader(f, delimiter="\t")
            for row in reader:
                if len(row) != 3:
                    continue
                key = row[-2].strip()
                val = row[-1].strip()
                if key and val:
                    try:
                        num_val = float(val)  # float can handle both ints and floats
                        fields[key] = num_val
                    except ValueError:
                        pass  # Ignore non-numeric values

        df = pd.DataFrame([fields])  # wrap in list to create a single-row DataFrame
        full_df = pd.concat([full_df, df], ignore_index=True)

    return full_df


df = load_results_dataframe(Path("../../results/"))
print(df.columns)
options = [col for col in df.columns if col not in ["Page Name", "Threads"]]
app = Dash(
    __name__,
    suppress_callback_exceptions=True,
    external_stylesheets=[dbc.themes.JOURNAL],
)

app.layout = dbc.Container(
    [
        dbc.Col(
            [
                dcc.Dropdown(
                    options=options,
                    value="CPI Rate",
                    id="dropdown-variable",
                    clearable=False,
                ),
                dcc.Graph(id="line-chart"),
            ]
        ),
    ],
    className="px-5 py-3 mb-5",
    fluid=True,
)


@callback(
    Output("line-chart", "figure"),
    Input("dropdown-variable", "value"),
)
def update_graphs_month(variable):
    # Line chart
    df_line = df.groupby(["Page Name", "Threads"], as_index=False)[variable].sum()
    line_fig = px.line(
        df_line, x="Threads", y=variable, color="Page Name", markers=True
    )
    return line_fig


if __name__ == "__main__":
    app.run(debug=True)
