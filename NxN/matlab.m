% Edge connections and corners
edges = [
    0 1
    1 2
    0 3
    1 4
    2 5
    3 4
    4 5
    3 6
    4 7
    5 8
    6 7
    7 8
];
edges = edges + 1;
corners = [1, 7, 9, 3];

function R = ProcessRaw(raw, resistors, probes)
    % Constants
    FixedG = 1/330;
    SupplyV = 5;
    resistorCount = size(resistors, 1);

    %Trim prefix and suffix
    raw = raw.replace("!", "");

    %Split into corner configs and voltage measurements
    states = split(raw, ":");
    splitStates = split(states, "|");
    config = str2double(split(splitStates(:,1), ","));
    voltages = str2double(split(splitStates(:,2), ","));
    nodes = size(voltages, 2);

    % Populate matrixes
    A = zeros([size(states, 1)*nodes resistorCount]);
    B = zeros([1, size(states, 1)*nodes 1]);

    for s=1:size(states, 1)
        voltage = voltages(s,:)';
        coeffs = zeros(nodes, resistorCount);

        for n=1:nodes
            for k = 1:resistorCount
               c1 = resistors(k, 1);
               c2 = resistors(k, 2);
               if n == c1
                   coeffs(n, k) = voltage(c1) - voltage(c2);
               elseif n == c2
                   coeffs(n, k) = voltage(c2) - voltage(c1);
                end
            end

            [isExciter, idx] = ismember(n, probes);
            if isExciter &&  config(s, idx) ~= -1
                vPin = config(s, idx) * SupplyV;
                B((s-1)*nodes+n) = FixedG * (vPin - voltage(n));
            else
                B((s-1)*nodes+n) = 0;
            end
        end

        A((s-1)*nodes+1:s*nodes,:) = coeffs;
    end

    G = lsqnonneg(A, B');
    R = (1 ./ G);
end

port = "YOUR PORT";
baud = 9600;
s = serialport(port, baud);
configureTerminator(s, "LF");
flush(s);

ys = [];
for l=1:100
    line = "";
    while ~line.startsWith("!")
        line = readline(s);
    end
    raw = replace(line, "\n", "");

    ys(size(ys, 1)+1,:) = ProcessRaw(raw, edges, corners)';
    plot(ys);
    legend("0-1", "1-2", "0-3", "1-4", "2-5", "3-4", "4-5", "3-6", "4-7", "5-8", "6-7", "7-8");
    axis([1 size(ys, 1) + 1 0 11000]);
    drawnow;
end

