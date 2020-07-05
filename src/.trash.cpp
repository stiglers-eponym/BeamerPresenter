/// Find the connected region in a list of integers around a start value.
/// Return the values of the gaps limiting the connected region.
/// List must be sorted and contain every value only once.
QPair<int,int> findGaps(const int start, const QList<int> list)
{
    // Check if start is contained in the list.
    if (!list.contains(start))
        return {start, start};
    // Check if the full list is simply connected.
    if (list.last() == list.first() + list.length() - 1)
        return {list.first() - 1, list.last() + 1};

    // Now we need to work with indices.
    int const startIndex = list.indexOf(start);

    int left_gap;
    // Check if the list is simply connected left of start
    if (start - list.first() == startIndex) {
        left_gap = list.first() - 1;
    }
    else if (startIndex < 2) {
        left_gap = list.first();
    }
    else {
        // Use a bisectoring algorithm to find the nearest gap to start
        int left = 0, right = startIndex, center;
        while (right > left + 1) {
            center = (right + left) / 2;
            if (start + center > startIndex + list[center])
                left = center;
            else
                right = center;
        }
        left_gap = list[left];
    }

    int right_gap;
    // Check if the list is simply connected right of start
    if (list.last() - start == list.length() - 1 - startIndex) {
        right_gap = list.last() + 1;
    }
    else if (list.length() < startIndex + 3) {
        right_gap = list.last();
    }
    else {
        // Use a bisectoring algorithm to find the nearest gap to start
        int left = startIndex, right = list.length() - 1, center;
        while (right > left + 1) {
            center = (right + left) / 2;
            if (start + center < startIndex + list[center])
                right = center;
            else
                left = center;
        }
        right_gap = list[right];
    }
    return {left_gap, right_gap};
}
